(function ($) { 
	
  Drupal.behaviors.vlot = {
    attach: function(context, settings) {
	    
     

	
      /* Our processing, we take an adjacency graph from php*/
	
      var network = null;
      var vislegend = new vis.DataSet();
      var visnodes = new vis.DataSet();
      var visedges = new vis.DataSet();
      var initialGroups =[];
      var activeGroups = [];
      var container=null;
      var groupoptions = {};
      var nodeLabelscaling=true;
      $(settings.vlot).each(function () {
        if (this.graphId)
        {
          container = $('#'+this.graphId).get(0);
          
          for (var groupkey in this.graphStyle) {
            groupoptions[groupkey] = {
              shape: 'box',
              color: {
                background: this.graphStyle[groupkey].color,
                border: this.graphStyle[groupkey].color,
                highlight: {
                  border: 'red',
                  background: 'red',
                  fontColor: 'white',
                },
              },  
              fontColor: 'white',
              fontSize: 12,  
              radius:20,
            };
          }   
       
            
            
          
         
         
            // Build the group styles
      
              
            
          var totalnodes = 0;
          var nodesCountByType = {};
            
            
          
         
            for (var key in this.nodes) {
              if (this.nodes.hasOwnProperty(key)) {
                totalnodes++;
                //Set group
                if (groupoptions.hasOwnProperty(this.nodes[key]['node_cmodel'])) {
                    currentGroup = this.nodes[key]['node_cmodel'];  
                 }
                 else {
                     currentGroup = "default"; 
                 }
                if (this.nodes[key]['nodetype']=='current_node')
                  {
                    
                    groupoptions['current_node'] = groupoptions[this.nodes[key]['node_cmodel']];
                    groupoptions['current_node'].color.border = 'red';
                    
                  }
                 
                
                if (this.graphStyle[this.nodes[key]['node_cmodel']])
                {
                 currentNodeCMODEL=this.graphStyle[this.nodes[key]['node_cmodel']].cmodel_label;
                  if (nodeLabelscaling)
                  {
                    currentNodeLabel=this.graphStyle[this.nodes[key]['node_cmodel']].cmodel_label.charAt(0);
                    
                  }
                  else
                  {
                    currentNodeLabel=this.graphStyle[this.nodes[key]['node_cmodel']].cmodel_label;	
                  }
                }
                else
                {
                  
                  currentNodeLabel =this.nodes[key]['node_cmodel'];
                  currentNodeCMODEL= currentNodeLabel;
                }
                //Add to active group array
                if (initialGroups.indexOf(currentGroup) == -1 ) {
                  initialGroups.push(currentGroup)
                 
                  
                  nodesCountByType[currentGroup]=0;
                
                }
                
                visnodes.add(
                  [{
                    id: key.replace( /(:|\.|\[|\])/g, "\\$1"),
                    label: currentNodeLabel,
                    cmodel:currentNodeCMODEL,
                    group: currentGroup,
                    title: this.nodes[key]["label"],
                    shape: 'box',
                    extlink: this.nodes[key]["link"],
                    nodetype:this.nodes[key]['nodetype'],
                  }
                ]);
                nodesCountByType[currentGroup]=nodesCountByType[currentGroup]+1;
                
                for (var relation in this.nodes[key]["relates_to"])
                {

                  for (var predicate in this.nodes[key]["relates_to"][relation])
                  {
                    visedges.add(
                      [{
                        from: key.replace( /(:|\.|\[|\])/g, "\\$1"),
                        to: relation.replace( /(:|\.|\[|\])/g, "\\$1" ),
                        label: '#'+this.nodes[key]["relates_to"][relation][predicate],
                        fontSize: 9,
               
                      }
                    ]);
                  }
                }
              } 
            }
            //Add active groups to legend
            activeGroups=initialGroups;
            for (var groupkey in activeGroups) {
            vislegend.add({
              id: activeGroups[groupkey],
                content: activeGroups[groupkey],
                options: groupoptions[activeGroups[groupkey]],
                nodescount: nodesCountByType[activeGroups[groupkey]],
              });
            }  
            
            
            
            // create a network
            var options = {
              //configurePhysics:true,
              physics: {
                barnesHut: {
                  enabled: true,
                  gravitationalConstant: -3000,
                  centralGravity: 0.05,
                  springLength: 95,
                  springConstant: 0.05,
                  damping: 0.3
                },
              },
              edges: {style:"arrow"},
              smoothCurves:true,
              stabilize: true,
              navigation: true,
              keyboard: false,
              hideEdgesOnDrag: true,
              groups: groupoptions 
            };
             console.log(totalnodes);
            if (totalnodes>150)
              {
                options.smoothCurves = false;
                //options.freezeForStabilization = true;
                options.clustering = false;
              }
              if (totalnodes>150) {
                options.stabilizationIterations = 5000;
              }
            //Let's create a dataview so we can filter out nodes by group  
            var visnodesFiltered = new vis.DataView(visnodes, {
                filter: function (item) {
                  return (jQuery.inArray(item.group,activeGroups) != -1);
                }
              });  
              
              var data = {
                nodes: visnodesFiltered,
                edges: visedges
              };
            
              
            network = new vis.Network(container, data, options);
            
            network.on('select', function(params) {
      				var newhtml= "";
              var allNodes = visnodes.get({returnType:"Object"});
              for (var nodekey in params.nodes)
                {
                  console.log(allNodes[params.nodes[nodekey]].nodetype);
                if (allNodes[params.nodes[nodekey]].nodetype == 'current_node') 
                  {
                   
                    newhtml = newhtml + "<li><span><i class=\"fa fa-info\">&nbspObject of type "+allNodes[params.nodes[nodekey]].cmodel+" </i></span><ul><li>&nbsp;This is your current Object: "+allNodes[params.nodes[nodekey]].title+"</li></ul></li>";
                  }
                else {
                 
                newhtml = newhtml + "<li><span><i class=\"fa fa-info\">&nbspObject of type "+allNodes[params.nodes[nodekey]].cmodel+" </i></span><ul><li><a href=\""+allNodes[params.nodes[nodekey]].extlink+"\"><i class=\"fa fa-eye\"></i> "+allNodes[params.nodes[nodekey]].title+"'s info</a></li><li>"+"<a href=\""+allNodes[params.nodes[nodekey]].extlink+"/dwc_related\"><i class=\"fa fa-sitemap\"></i> "+allNodes[params.nodes[nodekey]].title+"'s related objects</a></li></ul></li>"
                }
                }
                if (params.nodes.length>0)  
                  {
                  $("#vlot-info-id").html("<ul class=\"nav nav-pills nav-stacked\">"+ newhtml +"</ul>");  
                  }
                  else
                  {
                    $("#vlot-info-id").html("Click an Object to select");  
                  }
              return false
              });
              /**
               * this function fills the external legend with content using the getLegend() function.
               */
              function populateExternalLegend() {
                  var vislegendData = vislegend.get();
                  var legendDiv = document.getElementById("Legend");
                  legendDiv.innerHTML = "";

                  // get for all groups:
                  for (var i = 0; i < vislegendData.length; i++) {
                      // create divs
                      var containerDiv = document.createElement("div");
                      var iconDiv = document.createElement("div");
                      var descriptionDiv = document.createElement("div");

                      // give divs classes and Ids where necessary
                      containerDiv.className = 'legendElementContainer';
                      containerDiv.id = vislegendData[i].id + "_legendContainer"
                      iconDiv.className = "iconContainer";
                      descriptionDiv.className = "descriptionContainer";

                      // get the legend for this group.
                      var legend = nodeOptionsToSVG(vislegendData[i],20,20);

                      // append class to icon. All styling classes from the vis.css have been copied over into the head here to be able to style the
                      // icons with the same classes if they are using the default ones.
                      legend.icon.setAttributeNS(null, "class", "legendIcon");

                      // append the legend to the corresponding divs
                      iconDiv.appendChild(legend.icon);
                      descriptionDiv.innerHTML = legend.label;

                      // determine the order for left and right orientation
                     
                          descriptionDiv.style.textAlign = "right";
                          containerDiv.appendChild(descriptionDiv);
                          containerDiv.appendChild(iconDiv);
                     
                      // append to the legend container div
                      legendDiv.appendChild(containerDiv);

                      // bind click event to this legend element.
                      containerDiv.onclick = toggleGroups.bind(this,vislegendData[i].id);
                  }
              }
              
              function nodeOptionsToSVG(node, width, height) {
                var svgX = 2;
                var svgY = 2;
                var svg = document.createElementNS('http://www.w3.org/2000/svg',"svg");
                var svg_g=document.createElementNS('http://www.w3.org/2000/svg',"g");
              
                switch (node.options.shape) {
                    case 'box': var svgIcon = document.createElementNS("http://www.w3.org/2000/svg", "rect"); break;
                    case 'circle':  var svgIcon = document.createElementNS("http://www.w3.org/2000/svg", "circle"); break;
                    default: var svgIcon = document.createElementNS("http://www.w3.org/2000/svg", "ellipse"); break;
                 }  
                 svgIcon.setAttributeNS(null, "x", svgX);
                 svgIcon.setAttributeNS(null, "y", svgY);
                 svgIcon.setAttributeNS(null, "rx", node.options.radius/3);
                 svgIcon.setAttributeNS(null, "ry", node.options.radius/3);
                 svgIcon.setAttributeNS(null, "width", width);
                 svgIcon.setAttributeNS(null, "height", height);
                 svgIcon.setAttributeNS(null, "class", "outline");
                 svgIcon.setAttributeNS(null, "style", "fill:" + node.options.color.background+";stroke:"+node.options.color.border+";stroke-width:"+2);
                 var text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
                 text.textContent = node.nodescount;
                 var textsize=node.nodescount.toString().length;
                 text.setAttribute('x', parseInt(svgX+(width/2)-(2*textsize)));
                 text.setAttribute('y',  parseInt(svgY+3+(height/2)));
                 text.setAttribute('fill', '#FFFFFF');
                 

                 
                 svg_g.appendChild(svgIcon);
                 svg_g.appendChild(text); 
                 svg.appendChild(svg_g);
                 
                 
               return {icon: svg, label: node.content}; 
              }
              function  toggleGroups(groupId) {
                  // get the container that was clicked on.
                  var container = document.getElementById(groupId + "_legendContainer")
                
              
                if (jQuery.inArray(groupId,activeGroups)!= -1 ) {
                  activeGroups.splice(jQuery.inArray(groupId,activeGroups), 1);
                  //Refresh the dataset view
                  
                  visnodesFiltered.refresh();
                  network.freezeSimulation(true);
                  container.className = container.className + " grouphidden";
                }
                else {
                  activeGroups.push(groupId);
                  
                  visnodesFiltered.refresh();
                  network.freezeSimulation(false);
                  container.className = container.className.replace("grouphidden",""); 
                }
                  // if visible, hide
                  /*/if (graph2d.isGroupVisible(groupId) == true) {
                      groups.update({id:groupId, visible:false});
                      container.className = container.className + " hidden";
                  }
                  else { // if invisible, show
                      groups.update({id:groupId, visible:true});
                      container.className = container.className.replace("hidden","");
                  }*/
              }
              
              
              
              populateExternalLegend();  
          
          }
        });
      }
    }
  })(jQuery);