Office Hours creates a Field, that you can add to any entity (like a location,
a restaurant or a user) to represent "office hours" or "opening hours".

== UPGRADE WARNING ==
You MUST run update.php, when you upgrade from version 1.1 (or lower)
to a -dev version or version 1.2 (when available).

== FEATURES ==
The Drupal 7 version now provdes the following features: 
- For every instance, you can set default weekly office hours (multi-value).
- Feeds module support to import data. (See below for details.)

The widget provides:
- 'allowed hours' restrictions;
- input validation;
- use of either a 24 or 12 hour clock;
- using 1, 2 or even more 'time blocks' per day (thanks to jonhattan).

The formatter provides:
- an 'open now'/'closed now' indicator;
- options to group days (E.g., "Mon-Fri 12:00-22:00").
- options to display the 'office hours' any way you want. (See below for details.)

You can configure the formatter as follows:
- Add the field to an entity/node;
- Select the 'Office hours' formatter;
- Set the formatter details at /admin/structure/types/manage/NODE_TYPE/display/VIEW_MODE;
or
- Add the field to a view;
- Select the 'Office hours' formatter;
- Check the formatter settings of the field;

== FORMATTING THE HOURS ==
Using the customizable separators in the formatter settings, you can format the hours any way you want. 
- The formatter is default set up to show a nice table.
- To export the data to a Google Places bulk upload file, you can create a view,
  and set the formatter to generate the following data (for a shop that opens from Monday to Friday): 
    2:10:00:18:00,3:10:00:18:00,4:10:00:18:00,5:10:00:18:00,6:10:00:18:00,7:12:00:20:00

== USING VIEWS - FIELDS ==
Add the Field to any Views display, as you are used to do.
- To show only 1 day per row in a Views display: 
  - add the field to your View,
  - open the MULTIPLE FIELD SETTINGS section,
  - UNcheck the option 'Display all values in the same row',
  - make also sure you display 'all' values. (only valid if you have upgraded from 1.1 version.)

== USING VIEWS - FILTER CRITERIA ==
Only default (out-of-the-box) Views functionality is provided.
- To show only the entities that have a office hours: 
  - add the filter criterion "Content: Office hours (field_office_hours:day)" to your View,
  - set the filter option 'Operator' to 'is not empty',
- To show only the entities that have office hours for e.g., Friday: 
  - add the filter criterion "Content: Office hours (field_office_hours:day)" to your View,
  - set the filter option 'Operator' to 'is equal to',
  - set the filter option 'Value' to '5', or leave 'Value' empty and set 'Expose operator' to YES.
- To show only the entities that are open NOW: 
  This is not possible, yet. 

== USING VIEWS - SORT CRITERIA ==
Only default (out-of-the-box) Views functionality is provided.
- To sort the times per day, add the 'day' sort criterion. 

== IMPORTING WITH FEEDS MODULE ==
To import data with the Feeds module, the following columns can be used:
- day;
- hours/morehours from;
- hours/morehours to;
- hours/morehours from-to.

The day should be stated in full English name, or a day number where sunday = 0, monday=1, etc.
The hours can be formatted as hh:mm or hh.mm

I suppose Feeds Tamper can help to format the times and/or day to the proper format.

Here's an example file:
nid;weekday;Hours_1;Hours_2
2345;monday;11:00 - 18:01;
2345;tuesday;10:00 - 12:00;13:15-17.45
2383;monday;11:00 - 18:01;
2383;tuesday;10:00 - 12:00;13:15-17.45
