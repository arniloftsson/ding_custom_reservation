(function ($) {
  "use strict";

  // The form is loaded via ajax so only after an ajax call can we set focus.
  $(document).ajaxComplete(function (e, xhr, settings) {
    $('#edit-provider-options-interest-period').focus();

    var firstTabbable = $('.popupbar-close');
    var lastTabbable = $('.reserve-button');

    var keydownHandlerLast = function (e) {
      if ((e.which === 9 && !e.shiftKey)) {
          e.preventDefault();
          firstTabbable.focus();
      }
   }

   var keydownHandlerFirst = function (e) {
    if ((e.which === 9 && e.shiftKey)) {
      e.preventDefault();
      lastTabbable.focus();
    }
  };

    /*redirect last tab to first input*/
    lastTabbable.on('keydown', keydownHandlerLast);

    /*redirect first shift+tab to last input*/
    firstTabbable.on('keydown', keydownHandlerFirst);

    firstTabbable.click(function (e) {
      firstTabbable.unbind('keydown', keydownHandlerFirst);
      lastTabbable.unbind('keydown', keydownHandlerLast);
    });
  });
}(jQuery));
