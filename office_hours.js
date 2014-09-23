(function ($) {
  Drupal.behaviors.office_hours = {
    attach: function(context,settings) {
      $(".oh-hide", context).parent().hide();
      $(".oh-add-more-link", context).each(function (i) {
        $(this).parent().children("div.office-hours-block").hide();
        // If the previous row has an "Add more hours" link, and the office-hours-block is hidden, don't show this line.
        $this_tr = $(this).closest("tr");
        if ($this_tr.prev().find(".oh-add-more-link").length && !$this_tr.prev().find("div.office-hours-block:visible").length) {
          $this_tr.hide();
        }
      })
      .click(show_upon_click);
      fix_striping();

      // Function to show an office-hours-block, when user clicks "Add more hours".
      function show_upon_click() {
        $(this).hide();
        $(this).parent().children("div.office-hours-block").fadeIn("slow");
        // If the next for has an "add more hours" link, show it.
        $next_tr = $(this).closest("tr").next();
        if ($next_tr.find(".oh-add-more-link").length) {
          $next_tr.show();
        }
        fix_striping();
        return false;
      };

      // Function to traverse visible rows and apply even/odd classes.
      function fix_striping() {
        $('tbody tr:visible', context).each(function (i) {
          if (i % 2 == 0) {
            $(this).removeClass('odd').addClass('even');
          } else {
            $(this).removeClass('even').addClass('odd');
          }
        });
      }

    }
  };
 })(jQuery);
