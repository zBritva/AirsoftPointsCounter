int currentPower=5;  
int currentFireRate=100;
byte flag;   //флажок
int current_firemode = 0;


const int outputPin = 2;
const int buttonPin = 3;
const int firemodePin = 4;

const int firePowerPin = 0;
const int fireRatePin = 1;

const int rateMinDelay = 66; //15 shots per second
const int rateMaxDelay = 940; //1 shoths per second

int rateRange = rateMaxDelay - rateMinDelay;
int deviderRate = 255 / rateRange;

const int powerMinDelay = 1; //15 shots per second
const int powerMaxDelay = 5; //1 shoths per second

int deviderPower = 255 / powerMaxDelay;

void setup() {
  pinMode(outputPin,OUTPUT);  //сюда подключено реле
  pinMode(buttonPin,INPUT);  //считываем состояние кнопки
}

void loop() {
  currentPower = analogRead(firePowerPin) / deviderPower;
  currentFireRate = analogRead(fireRatePin) / deviderRate;
  
  //constraints of variable value
  if(currentPower < powerMinDelay)
    currentPower = powerMinDelay;
    
  if(currentPower > rateMaxDelay)
    currentPower = rateMaxDelay;
    
  if(currentFireRate < rateMinDelay)
    currentFireRate = rateMinDelay;
    
  if(currentFireRate > rateMaxDelay)
    currentFireRate = rateMaxDelay;  
  
  //shot off
  if(current_firemode == 0)
    return;
  
  if (digitalRead(buttonPin)==1 && (flag==0 || current_firemode == 2)) {   //если кнопка нажата и флаг опущен
    digitalWrite(outputPin,HIGH);  //открыть клапан
    delay(currentPower);   //подождать чутка
    digitalWrite(outputPin,LOW);  //закрыть клапан
    flag=1;  //поднять паруса! Ой, флаг
    delay(currentFireRate);   //защита от дребезга
  }

  if (digitalRead(buttonPin)==0 && flag==1) {  //если кнопка отпущена и поднят флаг (былсовершён выстрел)
    flag=0;  //опустить флаг
  }
  delay(2);  //задержка для стабильности работы
}
