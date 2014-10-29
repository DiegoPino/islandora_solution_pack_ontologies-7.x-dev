(function ($) { 
	
Drupal.behaviors.vlot = {
	attach: function(context, settings) {
	    
		var Renderer = function(canvas){
		  var dom=$(canvas);
		  var canvas = dom.get(0)
	      var ctx = canvas.getContext("2d");
	      var gfx = arbor.Graphics(canvas)
	      var particleSystem = null

	      var that = {
	        init:function(system){
	          particleSystem = system
	          particleSystem.screenSize(canvas.width, canvas.height) 
	          particleSystem.screenPadding(60);
	          that.initMouseHandling()
	        },

	        redraw:function(){
	          if (!particleSystem) return
			  if (particleSystem.energy().mean<=0.04) return
			  gfx.clear() // convenience ƒ: clears the whole canvas rect

	          // draw the nodes & save their bounds for edge drawing
	          var nodeBoxes = {}
			 
	          particleSystem.eachNode(function(node, pt){
	            // node: {mass:#, p:{x,y}, name:"", data:{}}
	            // pt:   {x:#, y:#}  node position in screen coords
                
				
	            // determine the box size and round off the coords if we'll be 
	            // drawing a text label (awful alignment jitter otherwise...)
	            var label = node.data.label||"Object"
				ctx.font = "12px Helvetica"
	            var w = ctx.measureText(""+label).width + 15
				
				var h = 0;
				if (node.data.nodetype=='current_node')
					{
					h = 16;	
					w =Math.max(w,ctx.measureText('(You are here)').width +15); 
					}
				
	            if (!(""+label).match(/^[ \t]*$/)){
	              pt.x = Math.floor(pt.x)
	              pt.y = Math.floor(pt.y)
	            }else{
	              label = null
	            }
                
	            // draw a rectangle centered at pt
	            if (node.data.color) ctx.fillStyle = node.data.color
	            else ctx.fillStyle = "rgba(0,0,0,.2)"
	            if (node.data.color=='none') ctx.fillStyle = "white"

	            if (node.data.shape=='dot'){
	              gfx.oval(pt.x-w/2, pt.y-w/2, w,w, {fill:ctx.fillStyle})
	              nodeBoxes[node.name] = [pt.x-w/2, pt.y-w/2, w,w]
	            }else{
				 
	              gfx.rect(pt.x-w/2, pt.y-10, w,20+h, 4, {fill:ctx.fillStyle})
	              nodeBoxes[node.name] = [pt.x-w/2, pt.y-11, w, 22+h]
				 
	            }

	            // draw the text
	            if (label){
	             
	              ctx.textAlign = "center"
	              ctx.fillStyle = "white"
	              if (node.data.color=='none') ctx.fillStyle = '#333333'
	              ctx.fillText(label||"", pt.x, pt.y+4)
	            }
				if (node.data.nodetype=='current_node')
					{
					ctx.font = "10px Helvetica"	
					ctx.fillText("(You are here)", pt.x, pt.y+18);	
					}
	          })    			


	          // draw the edges
	          particleSystem.eachEdge(function(edge, pt1, pt2){
	            // edge: {source:Node, target:Node, length:#, data:{}}
	            // pt1:  {x:#, y:#}  source position in screen coords
	            // pt2:  {x:#, y:#}  target position in screen coords

	            var weight = edge.data.weight
	            var color = edge.data.color
				var showText =edge.data.showText;
	            if (!color || (""+color).match(/^[ \t]*$/)) color = null

	            // find the start point
	            var tail = intersect_line_box(pt1, pt2, nodeBoxes[edge.source.name])
	            var head = intersect_line_box(tail, pt2, nodeBoxes[edge.target.name])

	            ctx.save() 
	              ctx.beginPath()
	              ctx.lineWidth = (!isNaN(weight)) ? parseFloat(weight) : 1
	              ctx.strokeStyle = (color) ? color : "#cccccc"
	              ctx.moveTo(tail.x, tail.y)
	              ctx.lineTo(head.x, head.y)
	              ctx.stroke()
				  if (showText)
				  	{
	              	ctx.font = '11px Helvetica';
                    var textmetric=ctx.measureText(edge.data.label); 
				    var dx = head.x-tail.x, dy=head.y-tail.y, len=Math.sqrt(dx*dx+dy*dy);
				    var tx = ((tail.x + head.x) / 2); 
				    var ty = ((tail.y + head.y) / 2); 
				    ctx.translate(tx,ty);		  
				    ctx.fillStyle = "black"; 
				    ctx.rotate(Math.atan2(dy,dx));
				    ctx.fillText (edge.data.label, 0, -5,textmetric.width);
				    }
				  
				  
				  
				  
				  
	            ctx.restore()

	            // draw an arrowhead if this is a -> style edge
	            if (edge.data.directed){
	              ctx.save()
	                // move to the head position of the edge we just drew
	                var wt = !isNaN(weight) ? parseFloat(weight) : 1
	                var arrowLength = 6 + wt
	                var arrowWidth = 2 + wt
	                ctx.fillStyle = (color) ? color : "#cccccc"
	                ctx.translate(head.x, head.y);
	                ctx.rotate(Math.atan2(head.y - tail.y, head.x - tail.x));

	                // delete some of the edge that's already there (so the point isn't hidden)
	                ctx.clearRect(-arrowLength/2,-wt/2, arrowLength/2,wt)

	                // draw the chevron
	                ctx.beginPath();
	                ctx.moveTo(-arrowLength, arrowWidth);
	                ctx.lineTo(0, 0);
	                ctx.lineTo(-arrowLength, -arrowWidth);
	                ctx.lineTo(-arrowLength * 0.8, -0);
	                ctx.closePath();
	                ctx.fill();
	              ctx.restore()
	            }
	          })



	        },
			
			
			
			
	        initMouseHandling:function(){
	          // no-nonsense drag and drop (thanks springy.js)
	          selected = null;
			  hovered =null;
	          nearest = null;
			  hovered_old =null;
	          var dragged = null;
	          var oldmass = 1
			  //Makes html links of node data for vlot clicked 
			 var htmlNodeLink = function(link,label)
				{
				
				$("#vlot-info-id").html("<span>Explore:</span><ul class=\"nav nav-pills nav-stacked\"><li><a href=\""+link+"\"><i class=\"fa fa-eye\"></i> "+label+"'s info</a></li><li>"+"<a href=\""+link+"/dwc_related\"><i class=\"fa fa-sitemap\"></i> "+label+"'s related objects</a></li>");	
				return false
				};
			  
			  var handler = {
				  over:function(e){
				  var old_nearest = nearest && nearest.node
	              var pos = $(canvas).offset();
	              _mouseP = arbor.Point(e.pageX-pos.left, e.pageY-pos.top)
				 
				  var prenear=particleSystem.nearest(_mouseP);
				 
				  if (prenear)
				  	{
				  
				  if (prenear.distance > 50) 
				  		{ 
						if (hovered_old)
							{		
								hovered_old.node.fixed = false;						
								hovered_old.node.mass=oldmass;
								//hovered_old.node.data.color=hovered_old.node.data.color_original;
								hovered_old.node.data.label=hovered_old.node.data.label_short;
		   						oldedges = particleSystem.getEdgesFrom(hovered_old.node);
		   						$.map(oldedges, function(edge){
		   							edge.data.showText=false;		
									});
									hovered_old=null;
									hovered=null;
									
							}
					 
					   return false; 
				   		
				   }
				  /*if (prenear.distance < 10) 	
				  	  {
					  selected=prenear;	
	                  dom.addClass('linkable');
	                  window.status = selected.node.data.link;	
				      }*/
				     }
				  
				  hovered = nearest = prenear;
                  if ((!nearest)|| (hovered==hovered_old) || nearest>10) return;
	              if (!hovered_old)
				  		{
						
	              	    hovered_old=hovered;
	  				    hovered.node.fixed = true;
	  				    //hovered.node.data.color='red';	
						
						oldmass=hovered.node.mass;
						hovered.node.data.label=hovered.node.data.label_full;
						hovered.node.mass=hovered.node.data.maxMass;
						 thisedges = particleSystem.getEdgesFrom(hovered.node);
						 $.map(thisedges, function(edge){
							 edge.data.showText=true;
						 });
						 return false;
						}
				  else
				  		{
				  		if 	(hovered!==hovered_old)
							{
			  				hovered_old.node.fixed = false;
						
							hovered.node.fixed = true;
							hovered_old.node.mass=oldmass;
							
							
							oldmass=hovered.node.mass;
							hovered.node.mass=hovered.node.data.maxMass;
							//hovered_old.node.data.color=hovered_old.node.data.color_original;
							hovered_old.node.data.label=hovered_old.node.data.label_short;
   						    oldedges = particleSystem.getEdgesFrom(hovered_old.node);
   						    $.map(oldedges, function(edge){
   								edge.data.showText=false;
   						 	});
						
						 thisedges = particleSystem.getEdgesFrom(hovered.node);
						 $.map(thisedges, function(edge){
							 edge.data.showText=true;
						 });
							hovered.node.data.label=hovered.node.data.label_full;
		  				    
		  				   // hovered.node.data.color='red';	
							hovered_old=hovered;
							
							}
				  		}
						
	              return false
	            },				
			clicked:function(e){
	              var pos = $(canvas).offset();
	              _mouseP = arbor.Point(e.pageX-pos.left, e.pageY-pos.top)
	              nearest = dragged = particleSystem.nearest(_mouseP);
                  
	              if (dragged.node !== null) dragged.node.fixed = true
                  $(canvas).unbind('mousemove', handler.over)
	              $(canvas).bind('mousemove', handler.dragged)
	              $(canvas).bind('mouseup', handler.dropped)

	              return false
	            },
	            dragged:function(e){
	              var old_nearest = nearest && nearest.node._id
	              var pos = $(canvas).offset();
	              var s = arbor.Point(e.pageX-pos.left, e.pageY-pos.top)

	              if (!nearest) return
	              if (dragged !== null && dragged.node !== null){
	                var p = particleSystem.fromScreen(s)
	                dragged.node.p = p
	              }

	              return false
	            },

	            dropped:function(e){
	              if (dragged===null || dragged.node===undefined) return
	              if (dragged.node !== null) dragged.node.fixed = false
                  if (dragged.node.data.link.length>0)
				  	{
					if ((selected) && (selected!=dragged)){
						selected.node.data.color=selected.node.data.color_original;
					}	
					selected=dragged;
					selected.node.data.color='red';	
					htmlNodeLink(dragged.node.data.link,dragged.node.data.label_full);	
				  	}
				  
				  dragged = null
	              //selected = null
	              $(canvas).unbind('mousemove', handler.dragged)
	              $(canvas).unbind('mouseup', handler.dropped)
				  $(canvas).bind('mousemove', handler.over)
	              
				  _mouseP = null
	              return false
	            },
				
			
			
				
				
	          }
	          $(canvas).mousedown(handler.clicked);
			  $(canvas).mousemove(handler.over);
		
	        }

	      }
		 
		
	      // helpers for figuring out where to draw arrows (thanks springy.js)
	      var intersect_line_line = function(p1, p2, p3, p4)
	      {
	        var denom = ((p4.y - p3.y)*(p2.x - p1.x) - (p4.x - p3.x)*(p2.y - p1.y));
	        if (denom === 0) return false // lines are parallel
	        var ua = ((p4.x - p3.x)*(p1.y - p3.y) - (p4.y - p3.y)*(p1.x - p3.x)) / denom;
	        var ub = ((p2.x - p1.x)*(p1.y - p3.y) - (p2.y - p1.y)*(p1.x - p3.x)) / denom;

	        if (ua < 0 || ua > 1 || ub < 0 || ub > 1)  return false
	        return arbor.Point(p1.x + ua * (p2.x - p1.x), p1.y + ua * (p2.y - p1.y));
	      }

	      var intersect_line_box = function(p1, p2, boxTuple)
	      {
	        var p3 = {x:boxTuple[0], y:boxTuple[1]},
	            w = boxTuple[2],
	            h = boxTuple[3]

	        var tl = {x: p3.x, y: p3.y};
	        var tr = {x: p3.x + w, y: p3.y};
	        var bl = {x: p3.x, y: p3.y + h};
	        var br = {x: p3.x + w, y: p3.y + h};

	        return intersect_line_line(p1, p2, tl, tr) ||
	              intersect_line_line(p1, p2, tr, br) ||
	              intersect_line_line(p1, p2, br, bl) ||
	              intersect_line_line(p1, p2, bl, tl) ||
	              false
	      }

	      return that
	    }   
	   
		
	
	
	
	
	
	
		/* Our processing, we take an adjacency graph from php*/
	
	$(settings.vlot).each(function () {
		var default_node_style;
		
		if (this.graphId)
			{
				if (!this.graphStyle)
					{
					default_node_style={'color':'grey','color_original':'grey','shape':'rect','label':'Object'};	
					}
			
			
			var maxnodesbeforescaling=this.graphNodeCountLimit || 30;
			
			var nodeLabelscaling=false;
		    var currentNodeLabel="Object";
			
			if (Object.keys(this.nodes).length>10)
				{
				var sys = arbor.ParticleSystem({friction:0.5, stiffness:150, repulsion:1000,fps:30,precision:0.2})
				var maxMass=2; // create the system with sensible repulsion/stiffness/friction
				var mass=1;
				var edgelength=1;
				}
			else
				{
			     var sys = arbor.ParticleSystem({friction:5, stiffness:256, repulsion:300})
				 var maxMass=2;// create the system with sensible repulsion/stiffness/friction
				 var mass=0.5;
				 var edgelength=0.5;
				}
			 sys.parameters({gravity:true})
			
			 // create the system with sensible repulsion/stiffness/friction
			sys.renderer = Renderer('#'+this.graphId) // our newly created renderer will have its .init() method called shortly by sys...
            // use center-gravity to make the graph settle nicely (ymmv)
		     
			
			 if (maxnodesbeforescaling<Object.keys(this.nodes).length){nodeLabelscaling=true;maxMass=3}
			 
			 
			 
			 for (var key in this.nodes) {
			    if (this.nodes.hasOwnProperty(key)) {
					
					if (this.graphStyle[this.nodes[key]["node_cmodel"]])
						{
						if (nodeLabelscaling)
							{
							currentNodeLabel=this.graphStyle[this.nodes[key]["node_cmodel"]].cmodel_label.charAt(0);	
							}
						else
							{
							currentNodeLabel=this.graphStyle[this.nodes[key]["node_cmodel"]].cmodel_label;	
							}
					    sys.addNode(key.replace( /(:|\.|\[|\])/g, "\\$1") ,{'color':this.graphStyle[this.nodes[key]["node_cmodel"]].color,'color_original':this.graphStyle[this.nodes[key]["node_cmodel"]].color,'shape':'rect','label':currentNodeLabel,'label_full':this.nodes[key]["label"],'label_short':currentNodeLabel,'mass':mass,'link':this.nodes[key]['link'],'nodetype':this.nodes[key]['nodetype'],'maxMass':maxMass});	
						}
					else
						{
							
						sys.addNode(key.replace( /(:|\.|\[|\])/g, "\\$1") ,{'color':'green','color_original':'green','shape':'rect','label': 'Object','label_full':this.nodes[key]["label"],'label_short':'O','link':this.nodes[key]['link'],'nodetype':this.nodes[key]['nodetype'],'mass':mass,'maxMass':maxMass});	
						}
					
			    }
			 }
			 for (var key in this.nodes) {
				
			    if (this.nodes.hasOwnProperty(key)) {
			        for (var relation in this.nodes[key]["relates_to"])
						{
					
						/*g.addEdge(key.replace( /(:|\.|\[|\])/g, "\\$1" ), relation.replace( /(:|\.|\[|\])/g, "\\$1" ), {directed: true});*/
						for (var predicate in this.nodes[key]["relates_to"][relation])
							{
							sys.addEdge(key.replace( /(:|\.|\[|\])/g, "\\$1" ),relation.replace( /(:|\.|\[|\])/g, "\\$1" ),{directed:true,length:edgelength, pointSize:3,label:'#'+this.nodes[key]["relates_to"][relation][predicate],showText:false})	
								
							}
						
						}
		
				   
					
			    }
			 }
	        
  		   		
		
		/*setInterval(function () {
		    if ($(context).length && $(context)[0].nodeName == '#document') {
		      NetworkGraph.render();
		    }
		  }, 100); */
			}
		
		
		
		
	});
	}
}
})(jQuery);