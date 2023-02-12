#include <Keypad.h>
#include <LiquidCrystal.h>
int stato_porta = 0;
int stato_arma = 0;
long tempo_precedente = 0; // variabile necessaria per la porta
int SENSORE = 0; //fotoresistenza
int valore; //valore rilevato
int luce_est = 22;
int SOGLIA = 700; //soglia per stabilire buio o luce
int campanello = 9;
int Pulsante = 24;
int rilevamento = 0; 
int movimento = 28; //sensore di movimento
int allarme = 2;
bool abilitata = false;
int pos = 90; //spostamento della porta
char combinazione[] = {'0', '4', '0', '7', '0', '4'};
int cont = 0; //conta i numeri inseriti per la combinazione;
LiquidCrystal lcd (27, 37, 33, 41, 36, 40);
/*LCD VSS al ground
  LCD VDD a 5V
  LCD con resistenza da qualche Kohm(1-2 Kohm) al ground
  LCD RS al pin 27
  LCD R/W al ground
  LCD Enable al pin 37
  LCD D4 al pin 33
  LCD D5 al pin 41
  LCD D6 al pin 36
  LCD D7 al pin 40
  LCD anodo del LED con resistenza (220-330 ohm) ai 5V
  LCD catodo del LED al ground*/

/*Codice letto = 42eff11f TESSERA
  Codice letto = 05af4fd3 CIP*/

char tastierino[4][4] = {
  {'1', '2', '3', 'A'}, //creo una matrice come il tastierino
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte righe[4] = {13, 11, 10, 8}; //pin delle righe
byte colonne[4] = {7, 6, 4, 3}; //pin delle colonne
Keypad customKeypad = Keypad(makeKeymap(tastierino), righe, colonne, 4, 4);

#include <MFRC522.h>
MFRC522 lettore(53, 5); 
#include <Servo.h>
Servo Elica;

String Tessere[] = {"05af4fd3" }; // IL CIP APRE, LA TESSERA NO
int Lunghezza = sizeof(Tessere) / sizeof(String); //la lunghezza del codice può anche cambiare, viene calcolata automaticamente

void setup() {
  lcd.begin(16, 2);
  pinMode(allarme, OUTPUT);
  pinMode(movimento, INPUT); //pin del sensore
  pinMode(luce_est, OUTPUT); //luce esterna
  pinMode(Pulsante, INPUT); //pulsante
  pinMode(campanello, OUTPUT);
  Serial.begin(9600);
  SPI.begin();
  Elica.attach(12);
  lettore.PCD_Init();
}

void arma_allarme() {
  //Serial.println("Ok");
  char tastoPremuto;
  bool combinazione_giusta = true;
  char digitata[6];
  //  Serial.println("stato arma");
  //  Serial.println(stato_arma);
  if (stato_arma == 0) { 
    tastoPremuto = customKeypad.getKey();
    //    Serial.println(tastoPremuto);
    if (tastoPremuto == '#') { 
      cont = 0;  //inizio a digitare
      //   Serial.println("vero");
      stato_arma = 1; // mando a uno e vedo prendo i valori della combinazione
      // tastoPremuto=false;
    }

  }
  if (stato_arma == 1) {
    if (cont < 6) {

      tastoPremuto = customKeypad.getKey();

      if (tastoPremuto && tastoPremuto != '#') { // se premo e premo diverso da # 
        //  Serial.println("ok");
        digitata[cont] = tastoPremuto; //digitata è un array dove inserisco i valori che digito
        //     Serial.println("cont");
        // Serial.println(cont);
        //  Serial.println(digitata[cont]);
        //   Serial.println("C");
        cont++;
      }
    } 
    else { //se ho messo 6 cifre
      stato_arma = 2; //mando a 2 e verifico la correttezza della combinazione
      cont = 0;
     // for (cont = 0; cont < 6; cont++) {
        //  Serial.println(cont);
        //   Serial.println(digitata[cont]);
      //}
    }
  }

  if (stato_arma == 2) {
    for (cont = 0; cont < 6; cont++) { //confronto quella inserita con quella che è la password
      //    Serial.println("i");
      //  Serial.println(cont);
      //  Serial.println(digitata[cont]);
      //    Serial.println(combinazione[cont]);
      //    Serial.println("-");
      if (digitata[cont] != combinazione[cont]) { //se anche un solo valore non coincide allora è falso
          combinazione_giusta = false;

        //     Serial.println("cg");
        //    Serial.println(combinazione_giusta);
      }
    }
    if (combinazione_giusta == true) { //se ho messo la cobinazione giusta
      //   Serial.println("corretta");
      if (abilitata == true) { // se era abilitata, disabilito e viceversa
        abilitata = false;
         lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("allarme");
        lcd.setCursor(0,1);
        lcd.print("disabilitata");
        Serial.println("disabilitata");
      }
      else {
        abilitata = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("allarme");
        lcd.setCursor(0,1);
        lcd.print("abilitata");
       Serial.println("abilitata");
      }
    }

    stato_arma = 0; //riporto tutto a 0
    cont = 0;
  }
}


String getCodice() {
  String uid = "";
  for (int i = 0; i < lettore.uid.size; i++) {
    uid += lettore.uid.uidByte[i] < 0x10 ? "0" : "";
    uid += String(lettore.uid.uidByte[i], HEX);
  }
  lettore.PICC_HaltA();
  return uid;
}

bool controllo(String chiave) { //si controlla se la tessera è nell’array dei codici
  for (int i = 0; i < Lunghezza; i++) {
    if (Tessere[i] == chiave) { //se c’è si restituisce vero
      return true;
    }
  }
  return false;
}

void porta() { //la prima volta che viene chiamata tempo precedente=0;
  long tempo = millis();
  // Serial.println(pos);
  //  if(pos==180 ){
  //    tempo_precedente=millis()+2000;
  //    Serial.println("millis con pos 180 = " + String(millis()) );
  //    Serial.println("tempo_precedente con pos 180 = " + String(tempo_precedente) );
  // }

  if (stato_porta == 1) { //apri
    if (millis() - tempo_precedente >= 10 && pos > 0) {
      pos--;
      Elica.write(pos);
      tempo_precedente = millis();
    }
    if (pos == 0) {
      stato_porta = -1;
      tempo_precedente = millis() + 2000;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("benvenut*");

    }

  } else if (stato_porta == -1) { //chiudi
    //Serial.println("tempo_precedente con porta -1 = " + String(tempo_precedente) );
    tempo = (millis() - tempo_precedente);
    //Serial.println("tempo con porta -1= " + String(tempo) );

    if (tempo >= 10 && pos < 90) {
      //    Serial.println(String(tempo) );
      pos++;
      Elica.write(pos);
      tempo_precedente = millis();
    }

    if (pos == 90) {
      tempo_precedente = 0; //riporto tutto a 0
      stato_porta = 0;
    }
  }
}

void lucefuori() {
  valore = analogRead(SENSORE); //luce esterna
 // Serial.println(valore);
  if (valore < SOGLIA) {
    digitalWrite(luce_est, HIGH);
  }
  else {
    digitalWrite(luce_est, LOW);
  }
}

void camp() {
  if (digitalRead(Pulsante) == HIGH){
    tone(campanello, 3000);
     lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DIN DON");
  }
  else
    noTone(campanello);
}


void allarm() {
  //arma_allarme();
  rilevamento = digitalRead(movimento); //lettura del sensore
  if (rilevamento == HIGH) { //se ha rilevato qualcosa
    Serial.println("Rilevato movimento ");
    //   abilitata=true;
    if (abilitata) {
      // arma_allarme();
      tone(allarme, 3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SOS!");
    }
    else {
      noTone(allarme);
    }
  }
  else {
    noTone(allarme);
  }
}

void loop() { //controllo se si è avvicinata una tessera e se è possibile leggere i dati
  //  Serial.println("Ok1");
  porta();
  lucefuori();
  camp();
  //Serial.println("Ok2");
  arma_allarme();
  // Serial.println("arma allarme");
  allarm();
  //  Serial.println("allarm");

  if (lettore.PICC_IsNewCardPresent() && lettore.PICC_ReadCardSerial()) {

    String codiceTessera = getCodice();
    if (controllo(codiceTessera)) { //se il codice è valido
      stato_porta = 1;
    }
  }
}
