#define BLYNK_TEMPLATE_ID           "TMPLG0TVgTHa"
#define BLYNK_DEVICE_NAME           "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "6i_gXmNegZnwAFt969MjK2xU6bzG7L7U"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Babe1";
char pass[] = "babe1234";

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#include <TimeLib.h>

BlynkTimer timer;



bool vibrate_set[1]={2};
long timer_start_set[1] = {0xFFFF};
long timer_stop_set[1] = {0xFFFF};
unsigned char weekday_set[1];

long rtc_sec;
unsigned char day_of_week;

bool vibrate_status[1]={2};
bool update_blynk_status[1]={2};
bool vibrate_timer_on_set[1]={2};

// Vibrate1 1
BLYNK_WRITE(V0)
{
  int val = param.asInt();

  if ( vibrate_timer_on_set[0] == 0 )
    vibrate_set[0] = val;
  else
    update_blynk_status[0] = 1;
}


BLYNK_WRITE(V1) 
{
  unsigned char week_day;
 
  TimeInputParam t(param);

  if (t.hasStartTime() && t.hasStopTime() ) 
  {
    timer_start_set[0] = (t.getStartHour() * 60 * 60) + (t.getStartMinute() * 60) + t.getStartSecond();
    timer_stop_set[0] = (t.getStopHour() * 60 * 60) + (t.getStopMinute() * 60) + t.getStopSecond();
    
    Serial.println(String("Start Time: ") +
                   t.getStartHour() + ":" +
                   t.getStartMinute() + ":" +
                   t.getStartSecond());
                   
    Serial.println(String("Stop Time: ") +
                   t.getStopHour() + ":" +
                   t.getStopMinute() + ":" +
                   t.getStopSecond());
                   
    for (int i = 1; i <= 7; i++) 
    {
      if (t.isWeekdaySelected(i)) 
      {
        week_day |= (0x01 << (i-1));
        Serial.println(String("Day ") + i + " is selected");
      }
      else
      {
        week_day &= (~(0x01 << (i-1)));
      }
    } 

    weekday_set[0] = week_day;
  }
  else
  {
    timer_start_set[0] = 0xFFFF;
    timer_stop_set[0] = 0xFFFF;
  }
}

BLYNK_WRITE(InternalPinRTC) 
{
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  unsigned long blynkTime = param.asLong(); 
  
  if (blynkTime >= DEFAULT_TIME) 
  {
    setTime(blynkTime);

    day_of_week = weekday();
  
    if ( day_of_week == 1 )
      day_of_week = 7;
    else
      day_of_week -= 1; 
    
    rtc_sec = (hour()*60*60) + (minute()*60) + second();
   
    Serial.println(blynkTime);
    Serial.println(String("RTC Server: ") + hour() + ":" + minute() + ":" + second());
    Serial.println(String("Day of Week: ") + weekday()); 
  }
}

// #########################################################################################################
BLYNK_CONNECTED() 
{
  Blynk.sendInternal("rtc", "sync"); 
}

// #########################################################################################################
void checkTime() 
{
  Blynk.sendInternal("rtc", "sync"); 
}

// #########################################################################################################
void vbt_mng()
{
  bool time_set_overflow;
  bool vbt_status_buf[3];
  
  for (int i=0; i<1; i++)
  {
    vbt_status_buf[i] = vibrate_status[i];
    time_set_overflow = 0;
    
    if ( timer_start_set[i] != 0xFFFF && timer_stop_set[i] != 0xFFFF)
    {
      if ( timer_stop_set[i] < timer_start_set[i] ) time_set_overflow = 1;

      if ((((time_set_overflow == 0 && (rtc_sec >= timer_start_set[i]) && (rtc_sec < timer_stop_set[i])) ||
        (time_set_overflow  && ((rtc_sec >= timer_start_set[i]) || (rtc_sec < timer_stop_set[i])))) && 
        (weekday_set[i] == 0x00 || (weekday_set[i] & (0x01 << (day_of_week - 1) )))) )
        {
          vibrate_timer_on_set[i] = 1;
        }
        else
          vibrate_timer_on_set[i] = 0;
    }
    else
      vibrate_timer_on_set[i] = 0;

    if ( vibrate_timer_on_set[i] )
    {
      vibrate_status[i] = 1;
      vibrate_set[i] = 0;
    }
    else
    {
      vibrate_status[i] = vibrate_set[i];
    }

    if ( vbt_status_buf[i] != vibrate_status[i] )
      update_blynk_status[i] = 1;  
  }
    // HARDWARE CONTROL
  digitalWrite(2, vibrate_status[0]);  

}

// #########################################################################################################
void blynk_update()
{
  if ( update_blynk_status[0] )
  {
      update_blynk_status[0] = 0;
      Blynk.virtualWrite(V0, vibrate_status[0]);
  }  

 
}

// #########################################################################################################
void setup()
{
 Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pinMode(2, OUTPUT);

  timer.setInterval(10000L, checkTime);
}

// #########################################################################################################
void loop() 
{
  timer.run();
  vbt_mng();
  blynk_update();
}
