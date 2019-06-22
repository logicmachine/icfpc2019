$(document).ready(function($){
  $('.clickable-table-row').on('click', function(){
    window.document.location = $(this).data('href');
  });
});
