let field_view = new FieldView($('#field-view')[0]);
let game_controller = null;


let initialize_state = (description) => {
  field_view.initialize(description);
  game_controller = new GameController(description, field_view);
};


$('#ctrl-task-file').on('click', function(){
  $('#task-file').click();
});


$('#task-file').on('change', function(e){
  // Get file object
  const files = e.target.files;
  if(files.length == 0){ return; }
  const file = files[0];
  // Update user interface
  $('#task-file-name').text(file.name);
  // Load and parse
  let reader = new FileReader();
  reader.onload = (e) => {
    initialize_state(new Description(e.target.result));
  };
  reader.readAsText(file);
});


let export_solution = () => {
  const s = game_controller.export_solution();
  $('#export-modal-content').text(s);
  $('#export-modal').modal();
  $('#export-modal-content').focus();
  $('#export-modal-content').select();
};

$('#ctrl-export-solution').on('click', export_solution);


$(document).on('keydown', function(e){
  const key = e.key;
  if(key == 'u'){
    game_controller.pop_record();
  }else if(key == 'w'){
    game_controller.move_up();
  }else if(key == 'a'){
    game_controller.move_left();
  }else if(key == 's'){
    game_controller.move_down();
  }else if(key == 'd'){
    game_controller.move_right();
  }else if(key == 'q'){
    game_controller.rotate_left();
  }else if(key == 'e'){
    game_controller.rotate_right();
  }
});
