(function ($) { 
	
  Drupal.behaviors.vlot = {
    attach: function(context, settings) {
	    
     

	
      /* Our processing, we take an adjacency graph from php*/
	
      var network = null;
      var visnodes = new vis.DataSet();
      var visedges = new vis.DataSet();
      var container=null;
      var groupoptions = {};
      var nodeLabelscaling=true;
      $(settings.vlot).each(function () {
        if (this.graphId)
        {
          container = $('#'+this.graphId).get(0);
          var data = {
            nodes: visnodes,
            edges: visedges
          };
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
       
            
            
          
          // add event listeners
         
            // Build the group styles
      
              
            
          var totalnodes= 0;
         
            for (var key in this.nodes) {
              if (this.nodes.hasOwnProperty(key)) {
                totalnodes++;
                if (this.nodes[key]['nodetype']=='current_node')
                  {
                    
                    groupoptions['current_node'] = groupoptions[this.nodes[key]['node_cmodel']];
                    groupoptions['current_node'].color.border = 'red';
                   // groupoptions['current_node']['fontSize'] = 13;
                    currentGroup = 'current_node';
                  }
                  else
                  {
                   currentGroup = this.nodes[key]['node_cmodel'];  
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
                
                visnodes.add(
                  [{
                    id: key.replace( /(:|\.|\[|\])/g, "\\$1"),
                    label: currentNodeLabel,
                    cmodel:currentNodeCMODEL,
                    group: currentGroup,
                    title: this.nodes[key]["label"],
                    shape: 'box',
                    extlink: this.nodes[key]["link"],
                  }
                ]);

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
            network = new vis.Network(container, data, options);
            network.on('select', function(params) {
      				var newhtml= "";
              var allNodes = visnodes.get({returnType:"Object"});
              for (var nodekey in params.nodes)
                {
                if (allNodes[params.nodes[nodekey]].group == 'current_node') 
                  {
                    newhtml = newhtml + "<li><i class=\"fa fa-eye\"></i>&nbsp;This is your current Object ("+allNodes[params.nodes[nodekey]].cmodel+"): "+allNodes[params.nodes[nodekey]].title+"</a></li>"
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
          }
        });
      }
    }
  })(jQuery);