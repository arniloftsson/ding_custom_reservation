(function ($) {
  "use strict";

  // The form is loaded via ajax so only after an ajax call can we set focus.
  $(document).ajaxComplete(function (e, xhr, settings) {
    $('#edit-provider-options-interest-period').focus();

    var firstTabbable = $('.popupbar-close');
    var lastTabbable = $('.reserve-button');

    /*redirect last tab to first input*/
    lastTabbable.on('keydown', function (e) {
       if ((e.which === 9 && !e.shiftKey)) {
         console.log("1");
           e.preventDefault();
           firstTabbable.focus();
       }
    });

    /*redirect first shift+tab to last input*/
    firstTabbable.on('keydown', function (e) {
        if ((e.which === 9 && e.shiftKey)) {
          console.log("2");
            e.preventDefault();
            lastTabbable.focus();
        }
    });
  });
}(jQuery));
