(function ($) { 
	
  Drupal.behaviors.vlot_manage = {
    attach: function(context, settings) {
      $('#visual-manage-relations', context).once('vlot-showmanage', function () {
      
      /* Our processing, we take an adjacency graph from php*/
      console.log(context);
      console.log(settings); 
      var network = null;
      var visnodes = new vis.DataSet();
      var visedges = new vis.DataSet();
      var container=null;
      var groupoptions = {};
      var nodeLabelscaling=true;
      
      $(settings.vlot_manage).each(function () {
        if (this.graphId) {
          container = $('#'+this.graphId).get(0);
          console.log(container);
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
            groups: groupoptions ,
            dataManipulation: true,
            onAdd: function(data,callback) {
              var span = document.getElementById('operation');
              var idInput = document.getElementById('node-id');
              var labelInput = document.getElementById('node-label');
              var saveButton = document.getElementById('saveButton');
              var cancelButton = document.getElementById('cancelButton');
              var div = document.getElementById('network-popUp');
              span.innerHTML = "Add Node";
              idInput.value = data.id;
              labelInput.value = data.label;
              saveButton.onclick = saveData.bind(this,data,callback);
              cancelButton.onclick = clearPopUp.bind();
              div.style.display = 'block';
            },
            onEdit: function(data,callback) {
              var span = document.getElementById('operation');
              var idInput = document.getElementById('node-id');
              var labelInput = document.getElementById('node-label');
              var saveButton = document.getElementById('saveButton');
              var cancelButton = document.getElementById('cancelButton');
              var div = document.getElementById('network-popUp');
              span.innerHTML = "Edit Node";
              idInput.value = data.id;
              labelInput.value = data.label;
              saveButton.onclick = saveData.bind(this,data,callback);
              cancelButton.onclick = clearPopUp.bind();
              div.style.display = 'block';
            },
            onConnect: function(data,callback) {
              if (data.from == data.to) {
                var r=confirm("Do you want to connect the node to itself?");
                if (r==true) {
                  callback(data);
                }
              }
              else {
                callback(data);
              }
            },
          };
          console.log(totalnodes);
          console.log(options);
          if (totalnodes>150)
          {
            options.smoothCurves = false;
            options.freezeForStabilization = true;
            options.clustering = false;
          }
          if (totalnodes>150) {
            options.stabilizationIterations = 5000;
          }
          
          network = new vis.Network(container, data, options);
            
        }
      });
    });
    }
  }
})(jQuery);