/*
**	/lib/date.c
**
**	This file contains functions to manipulate and denerate time
**	and dates on a variety of formats. For reference,
**
**		ctime(0) == Wed Dec 31 16:00:00 1969
**
**	Copyright Skotos Tech Inc 1999
*/


/* humanized_date(0) == 16:00 PM, December 31st, 1969 */

static
string humanized_date(int n) {
    int hour, minute, day, year;
    string month, ampm, res;

    if (sscanf(ctime(n), "%*s %s %d %d:%d:%*d %d",
	       month, day, hour, minute, year)) {
        if (hour < 12) {
	    ampm = "AM";
	} else {
	    ampm = "PM";
	}
	hour = hour % 12;
	if (hour == 0) {
	    hour = 12;
	}
	if (minute < 10) {
	    res = hour + ":0" + minute;
	} else {
	    res = hour + ":" + minute;
	}
	res += " " + ampm + ", ";
	switch(month) {
	case "Jan": month = "January";		break;
	case "Feb": month = "February";		break;
	case "Mar": month = "March";		break;
	case "Apr": month = "April";		break;
	case "May": month = "May";		break;
	case "Jun": month = "June";		break;
	case "Jul": month = "July";		break;
	case "Aug": month = "August";		break;
	case "Sep": month = "September";	break;
	case "Oct": month = "October";		break;
	case "Nov": month = "November";		break;
	case "Dec": month = "December";		break;
	}
	res += month + " " + day;
	switch(day) {
	case 1: case 21: case 31:
	    res += "st"; break;
	case 2: case 22:
	    res += "nd"; break;
	case 3: case 23:
	    res += "rd"; break;
	default:
	    res += "th"; break;
	}
	return res + ", " + year;
    }
    error("bizarre ctime");
}

/* rfc_1123_date(0) == Wed, 31 Dec 1969 16:00:00 GMT */
static
string rfc_1123_date(int n) {
    int date, year;
    string time, month, day, res;

    /* TODO HACK */
    n += 6*3600;
    if (sscanf(ctime(n), "%s %s %d %s %d",
	       day, month, date, time, year)) {
	res = day + ", ";
	if (date >= 10) {
	    res += date;
	} else {
	    res += "0" + date;
	}
	return res + " " + month + " " + year + " " + time + " GMT";
    }
    error("bizarre ctime");
}

static
string days(int n) {
    int hours, minutes;

    n /= 60;	 /* ignore seconds */
    minutes = n % 60;	n /= 60;
    hours = n % 24;	n /= 24;
    if (n == 0) {
	if (hours == 0) {
	    return minutes + " m";
	}
	return hours + " h, " + minutes + " m";
    }
    return n + " d, " + hours + " h";
}

/* Mon Aug 30 14:38:01 1999 */

static
string short_time(int stamp) {
   return ctime(stamp)[4..15];
}

static string
smalltime(int t) 
{
    return ctime(t)[11 .. 15];
}

static
int get_day(int time) {
   int d;

   sscanf(ctime(time)[8 .. 9], "%d", d);
   return d;
}

static
int get_month(int time) {
   switch(ctime(time)[4 .. 6]) {
   case "Jan":
      return 1;         break;
   case "Feb":
      return 2;         break;
   case "Mar":
      return 3;         break;
   case "Apr":
      return 4;         break;
   case "May":
      return 5;         break;
   case "Jun":
      return 6;         break;
   case "Jul":
      return 7;         break;
   case "Aug":
      return 8;         break;
   case "Sep":
      return 9;         break;
   case "Oct":
      return 10;        break;
   case "Nov":
      return 11;        break;
   case "Dec":
      return 12;        break;
   default:
      error("bizarre month in ctime");
   }
}

static
string get_month_full(int time)
{
   switch(ctime(time)[4 .. 6]) {
   case "Jan": return "January";
   case "Feb": return "February";
   case "Mar": return "March";
   case "Apr": return "April";
   case "May": return "May";
   case "Jun": return "June";
   case "Jul": return "July";
   case "Aug": return "August";
   case "Sep": return "September";
   case "Oct": return "October";
   case "Nov": return "November";
   case "Dec": return "December";
   default:
      error("bizarre month in ctime");
   }
}

static
string get_month_abbr(int time)
{
   return ctime(time)[4 .. 6];
}

static
int get_weekday(int time)
{
   switch (ctime(time)[.. 2]) {
   case "Sun": return 0;
   case "Mon": return 1;
   case "Tue": return 2;
   case "Wed": return 3;
   case "Thu": return 4;
   case "Fri": return 5;
   case "Sat": return 6;
   default:
      error("bizarre weekday in ctime");
   }
}

static
string get_weekday_full(int time)
{
   switch (ctime(time)[.. 2]) {
   case "Sun": return "Sunday";
   case "Mon": return "Monday";
   case "Tue": return "Tuesday";
   case "Wed": return "Wednesday";
   case "Thu": return "Thursday";
   case "Fri": return "Friday";
   case "Sat": return "Saturday";
   default:
      error("bizarre weekday in ctime");
   }
}

static
string get_weekday_abbr(int time)
{
   return ctime(time)[.. 2];
}

static
int get_year(int time) {
   return (int) ctime(time)[20 ..];
}

static
int get_hour(int time) {
   return (int) ctime(time)[11 .. 12];
}

static
int get_minute(int time) {
   return (int) ctime(time)[14 .. 15];
}

static
int get_second(int time) {
   return (int) ctime(time)[17 .. 18];
}

# define YEAR   (365 * 24 * 3600)
# define MONTH  ( 28 * 24 * 3600)
# define DAY    (      24 * 3600)
# define HOUR   (           3600)
# define MINUTE (             60)

/*
 * Needed something that wouldn't trip over daylightsaving:
 */
static
int calculate_stamp(int year, int month, int day, varargs int guess) {
   /*
    * Ignore the given guess and come up with our own.
    *
    * Guess should be an _underestimate_, otherwise all bets are off:
    */
   guess = (year - 1970) * YEAR + (month - 1) * MONTH + (day - 1) * DAY;

   /*
    * Now slowly move forward to the right year/month/day.
    */
   while (get_year(guess) != year || get_month(guess) != month || get_day(guess) != day) {
      guess += DAY;
   }

   /*
    * This takes care of the GMT-offset and daylight saving adjustments.
    */
   guess = guess - get_minute(guess) * MINUTE - get_hour(guess) * HOUR;

   DEBUG("calculate_stamp(" + year + ", " + month + ", " + day + ", ...) => " + guess + " (" + ctime(guess) + ")");
   return guess;
}
