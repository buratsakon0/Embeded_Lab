// LED ไว้ที่ขา 23
#define LED     23  
// LED ไว้ที่ขา 32
#define BUTTON  32

void setup() {
  pinMode(BUTTON, INPUT_PULLUP); // set ให้ button เป็น input pullup (ไม่ต้องต่อง R )
  pinMode(LED, OUTPUT);  // set ให้ LED เป็น output
}void loop() {
  if(digitalRead(BUTTON)==LOW) { 
    digitalWrite(LED, HIGH);
  }
}
