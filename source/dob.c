#include <stdio.h>
#include <string.h>
#include <time.h>


#include "server.h"

#define is_leap(year) ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0)) // For user birthday date


// Function for date converting to day order number in year
int ymd_to_ord(int year, int month, int day) {
	static int days_before_month[] = {
	0, /* Non-using. Months have numbers starting with 1 */
	0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 // month order number
	};
	
	int y = year - 1;
	return day + days_before_month[month] + (is_leap(year) && month > 2) + y * 365 + y / 4 - y / 100 + y / 400;
}


int birthday_evaluate(char* user_dob) {
	time_t current_time;
	time(&current_time);

	struct tm *local_time = localtime(&current_time);

	int curr_year = local_time->tm_year + 1900;
	int curr_month = local_time->tm_mon + 1;
	int curr_day = local_time->tm_mday;
	
	char dob_year[5] = {0};
	char dob_month[3] = {0};
	char dob_day[3] = {0};
	
	char* hyphen1 = strchr(user_dob, '-');
	char* hyphen2 = strchr(hyphen1 + 1, '-');
	
	strncpy(dob_year, user_dob, hyphen1 - user_dob);
	strncpy(dob_month, hyphen1 + 1, hyphen2 - hyphen1 - 1);
	strcpy(dob_day, hyphen2 + 1);
	
	int year = atoi(dob_year);
	int month = atoi(dob_month);
	int day = atoi(dob_day);
	
	
	//printf("Date of birthday: %d-%d-%d\n", year, month, day);
	
	int birthday_day = day;
	int birthday_month =  month;
	int birthday_year = year;
	
	if (birthday_month > curr_month || (birthday_month == curr_month && birthday_day > curr_day)) {
        	birthday_year = curr_year;
    	} else {
        	birthday_year = curr_year + 1;
    	}
    	
    	int birthday_ord = ymd_to_ord(birthday_year, birthday_month, birthday_day);
	int current_ord = ymd_to_ord(curr_year, curr_month, curr_day);

	int days_difference;

	if (birthday_day == curr_day && birthday_month == curr_month) {
		days_difference = 0;
	} else {
		days_difference = birthday_ord - current_ord;
	}

	//printf("Until days to birthday: %d\n", days_difference);

	return days_difference;
}
