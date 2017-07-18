//Author: Antonino Fazio - fazioa@gmail.com
//2017
//Lo sketch usa una shield SIM900A ed un display 16*2 caratteri
//Riceve esclusivamente SMS e li visualizza sul display
//Studiato come sistema di comunicazione box - pilota

//PIN SIM900A
//2 to TXD
//3 to RXD

//PIN Display I2C
//A4 to SDA
//A5 to SCL

#define TIMEOUT_MESSAGGIO 50 //tempo in secondi
#define SIGNALBAR //se la variabile è definita allora esegue il controllo del segnale
#define SEGNALE_OGNI_CICLI 5
#define TACCHE_SEGNALE_BASSO_DA_SOTTRARRE 5

#include "SIM900.h"
#include <SoftwareSerial.h>

//If you want to use the Arduino functions to manage SMS, uncomment the lines below.
#include "sms.h"
#include "caratteriBarraSegnale.h"
SMSGSM sms;



#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

boolean started = false;
char smsbuffer[160];
char n[20];
char sms_position;
char phone_number[20]; // array for the phone number string
char sms_text[32];
int i;
String sMessaggio;
int iWaitingDot = 0;
int iNextPosSMS;
String sSignal;
int iCount = 0;

void delete_sms_text_array() {
  for (int i = 0; i < 32; i++) {
    sms_text[i] = 0;
  }
}

void deleteAllSMS() {
  if (started)
  {
    for (i = 1; i <= 30; i++)
    {
      sms.DeleteSMS(i);
      Serial.print("Cancello SMS ");
      Serial.println(i);
    }
  }
}

long tempo;
void setStart_PersistenzaMsg() {
  tempo = millis();
}
boolean checkTime_PersistenzaMsg() {
  Serial.print("Timeout = ");
  Serial.println(TIMEOUT_MESSAGGIO, DEC);
  Serial.print("Tempo persistenza msg = ");
  Serial.println((int) ((millis() - tempo) / 1000));
  if (millis() - tempo > (long) TIMEOUT_MESSAGGIO * 1000) {
    return true;
  }
  return false;
}

void stampaBarra(long lSignal, int iCaratteriDisponibili) {

  double a = ((double) iCaratteriDisponibili / 100) * (((double)lSignal / 31) * 100);
  Serial.print("a: ");
  Serial.println(a);
  // disegna i rettangoli neri sull'lcd
  if (a >= 1) {
    for (int i = 1; i < a; i++) {
      lcd.write(4);
      b = i;
    }
    a = a - b;
  }
  peace = a * 5;
  // drawing charater's colums
  switch (peace) {
    case 0:
      break;
    case 1:
      lcd.print((char)0);
      break;
    case 2:
      lcd.write(1);
      break;
    case 3:
      lcd.write(2);
      break;
    case 4:
      lcd.write(3);
      break;
  }
  //clearing line

  for (int i = 0; i < (iCaratteriDisponibili - b); i++) {
    lcd.print(" ");
  }
}


void setup() {
  //Serial connection.
  Serial.begin(9600);
  Serial.println("EXTREME RIDERS Moto Messages Box");

  Serial.println("LCD Begin and Init");
  lcd.init();                      // initialize the lcd
  lcd.createChar(0, p1);
  lcd.createChar(1, p2);
  lcd.createChar(2, p3); //genere i caratteri personalizzati
  lcd.createChar(3, p4);
  lcd.createChar(4, p5);


  //crea il set di caratteri definiti nel file caratteriBarraSegnale.h
  Serial.println("LCD: Crea caratteri personalizzati per barra grafica");


  Serial.println("wait...");

  lcd.setCursor(1, 0);
  lcd.print("EXTREME RIDERS");
  lcd.setCursor(0, 1);
  lcd.print("wait...");

  lcd.backlight();



  //Start configuration of shield with baudrate.
  while (!started) {
    if (gsm.begin(4800))
    {
      Serial.println("\nstatus=READY");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Modulo GSM OK!");
      started = true;
    }
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Modulo GSM KO!");
      Serial.println("\nstatus=IDLE");
    }
  }
  if (started) {
    lcd.setCursor(0, 1);
    lcd.print("Start...");
    deleteAllSMS();
  }

  delete_sms_text_array();
  lcd.clear();
}


void loop() {


  if (started)
  {
    //Read if there are messages on SIM card and print them.
    sms_position = sms.IsSMSPresent(SMS_UNREAD);
    //sms_position = sms.IsSMSPresent(SMS_ALL);
    //sms_position = 19;
    //for (sms_position = 0; sms_position <= 50; sms_position++) {

    if (sms_position > 0) {
      lcd.clear();
      delete_sms_text_array();
      //avvia il timer. Dopo il TIMEOUT il messaggio verrà cancellato dal display
      setStart_PersistenzaMsg();
      // read new SMS
      Serial.print("SMS postion:");
      Serial.println(sms_position, DEC);
      sms.GetSMS(sms_position, phone_number, sms_text, 32);

      Serial.print("Cancello SMS ");
      Serial.println(sms_position, DEC);
      sms.DeleteSMS(sms_position);

      // now we have phone number string in phone_num
      Serial.print("SMS da: ");
      Serial.println(phone_number);
      Serial.print("Testo SMS: ");
      Serial.println(sms_text);

      //cancella il display
      lcd.clear();
      //linea 0
      lcd.setCursor(0, 0);
      //azzera la stringa
      sMessaggio = "";

      //divisione messaggio in due righe da 16 caratteri
      //prende i primi 16 caratteri di sms_text da stampare sulla riga 0
      for (int k = 0; k <= 15; k++) {
        if (sms_text[k] != 0) {
          //10 è il ritorno a capo
          if (sms_text[k] == 10) {
            iNextPosSMS = k + 1;
            k = 99;
          } else if (sms_text[k] == '\\') {
            if (sms_text[k + 1] == 'n') {
              //imposta l'indice per il carattere successivo, da visualizzare sulla seconda riga
              iNextPosSMS = k + 2;
              //imposta indice fittizio per uscire forzatamente dal ciclo
              k = 99;
            }
          } else {
            sMessaggio += sms_text[k];
            iNextPosSMS = 16;
          }
        }
      }
      //stampa il messaggio
      lcd.print(sMessaggio);
      Serial.print("1: ");
      Serial.println(sMessaggio);
      //azzera la stringa
      sMessaggio = "";
      //linea 0
      lcd.setCursor(0, 1);


      //prende i secondi 16 caratteri di sms_text da stampare sulla riga 1
      for (int k = iNextPosSMS; k < iNextPosSMS + 16; k++) {
        if (sms_text[k] != 0) {
          //          //se nella seconda linea trovo un carattere di controllo \ allora lo ignoro
          //          if (sms_text[k] == '\\') {
          //            k++;
          //          }
          sMessaggio += sms_text[k];
        }
      }
      lcd.print(sMessaggio);
      Serial.print("2: ");
      Serial.println(sMessaggio);

    } else
    {
      //  -1 - comm. line to the GSM module is not free
      //  -2 - GSM module didn't answer in timeout
      //  -3 - GSM module has answered "ERROR" string
      switch (sms_position)
      {
        case -1 : Serial.println("comm. line to the GSM module is not free");
          break;
        case -2 : Serial.println("GSM module didn't answer in timeout");
          break;
        case -3 : Serial.println("GSM module has answered ERROR string");
          break;
      }

      Serial.println("NO NEW SMS,WAITING");
      //controlla che l'array di char che contiene il messaggio sia vuoto
      boolean bIsEmpty = true;
      for (int i = 0; i < 32; i++) {
        if (sms_text[i] != 0) bIsEmpty = false;
      }
      if (bIsEmpty) {
        //se non c'è alcun messaggio da visualizzare allora il display mostra la schermata di attesa
        //lcd.clear();
        lcd.setCursor(iWaitingDot, 0);
        lcd.print(" ");
        iWaitingDot = ++iWaitingDot % 16;
        lcd.setCursor(iWaitingDot, 0);
        lcd.print(".");

#ifdef SIGNALBAR
        //stampa la barra del segnale ogni cinque cicli
        iCount++;
        if (iCount % SEGNALE_OGNI_CICLI == 0) {
          //se il sistema è in attesa allora visualizza l'intensità del segnale
          gsm.SimpleWriteln("AT+CSQ");
          //gsm.SimpleWriteln("AT+CSQ=?");

          // gsm.WhileSimpleRead();
          long lSignal = gsm._tf.getValue();

          Serial.print("Signal (0..31): ");
          Serial.println(lSignal);



          lcd.setCursor(0, 1);
          lcd.print("s:");
          //stampa sulla seconda riga del display una barra che indica l'intensità del segnale
          lcd.setCursor(2, 1);

          //sottraggo dal segnale il valore TACCHE_SEGNALE_BASSO_DA_SOTTRARRE, corrispondenti al segnale eccessivamente basso.
          lSignal = lSignal - TACCHE_SEGNALE_BASSO_DA_SOTTRARRE;

          //stampa la barra grafica del segnale gsm sul display
          //parametri: valore percentuale, caratteri disponibili sul display
          stampaBarra( lSignal, 13);
#endif
        }
      } else {
        //se c'è un messaggio in visuallizzazione allora controllo da quanto tempo è visualizzato. Trascorso il tempo di TIMEOUT il msg viene cancellato
        if (checkTime_PersistenzaMsg()) {
          Serial.println("Timeout - Cancellazione messaggio dal display");
          //cancella il display
          lcd.clear();
          //cancella il contenuto dell'array messaggio. in questo modo il display mostrerà la schermata di attesa msg
          delete_sms_text_array() ;
        }
      }
    }
    delay(1000);
  }
}



