// Controlador de Levitador Elementar - UNO / Nano
// Parte do projeto Experimento com Levitadores
// https://sites.google.com/usp.br/explev

// A frequencia, f, sera 16000kHz/(2*(iMax+1)), pois
// o clock do Arduino UNO/Nano eh 16MHz (=16000kHz)
// e o contador comeca em zero e inverte a polaridade
// da saida quando atinge iMax, de modo que o periodo
// do sinal de saida corresponde a duas inversoes que
// ocorrem a cada iMax+1 clocks. Para calcular o iMax
// para uma dada frequencia usar iMax=(8000kHz/f)-1
// Com emissores de f=40kHz, usar iMax=199
// Com emissores de f=25kHz, usar iMax=319
//int iMax = 199; // para f=40kHz
int iMax = 319; // para f=25kHz

// define o pino digital que controlará a alimentacao 
// dos emissores (o sinal de EnA da ponte-H).
// So nao pode ser um dos pinos do Timer1 (9 e 10) 
// nem os pinos de comunicacao serial
byte pinoEnA = 8;

// Parametros iniciais
bool potenciaAtual = true;

void setup() {
  Serial.begin(250000);
  Serial.println( F("Controlador Elementar - ExpLev") );
  expLevShowHelp(0);
  Serial.setTimeout( 200 );
  expLevConfiguraTimer1();
  pinMode( pinoEnA, OUTPUT );
  digitalWrite( pinoEnA, potenciaAtual ); 
}

void loop() {
  if (Serial.available()){
    char proxChar = Serial.read();
    if (proxChar==10 || proxChar==13 ||
    proxChar==32 || proxChar==','){
      // Nada a fazer, sao caracteres a serem ignorados        
    }else if ( proxChar=='p' || proxChar=='P'){
      int valor = Serial.parseInt();
      if (valor>=0 && valor<=1){
        if (valor==0){
          potenciaAtual=false;
        }else if(valor==1){
          potenciaAtual=true;
        }
        digitalWrite( pinoEnA, potenciaAtual );
        expLevShowHelp(-1);
      }else{
        Serial.println( F("Opcao invalida") );
      }
    }else if ( proxChar=='o' || proxChar=='O' ){
      float Dt = Serial.parseFloat();
      if (Dt>0){
        expLevComandoO(Dt);
      }else{
        Serial.println( F("Opcao invalida") );
      }
    }else if( proxChar=='c' || proxChar=='C' ){
      long nRep = Serial.parseInt();
      float DtRep = Serial.parseFloat();
      float Dt1 = Serial.parseFloat();
      if ( nRep>0 && DtRep>0 && Dt1>0 && DtRep>Dt1 ){
        expLevComandoC(nRep, DtRep, Dt1);
      }else{
        Serial.println( F("Opcao invalida: ") );
      }
    }else if( proxChar=='s' || proxChar=='S' ){
      expLevShowHelp(-1);
    }else if( proxChar=='?' ){
      expLevShowHelp(0);
    }else if( proxChar=='h' || proxChar=='H' ){
      expLevShowHelp(1);
    }else if( proxChar=='i' || proxChar=='I' ){
      int newIMax = Serial.parseInt();
      if (newIMax>100){
        iMax = newIMax;
        expLevConfiguraTimer1();
        expLevShowHelp(-1);
      }
    }else{
      Serial.print( F("Comando invalido") );
      Serial.println( proxChar );
    }      
  }
}


void expLevConfiguraTimer1(){
  // Definir os pinos associados aos comparadores como output
  pinMode( 9, OUTPUT ); // OC1A = 9
  pinMode( 10, OUTPUT ); // OC1B = 10
 
  // Parar os os timers do Arduino
  GTCCR = 0b10000011; // na base decimal é 131
  
  // Definir o timer 1 como CTC com TOP no OCR1A
  // Definir o prescaler do Timer 1 para 1
  TCCR1A = 0b01010000; // 80
  TCCR1B = 0b00001001; // 9
  
  // Definir os OCR1A e OCR1B para iMax
  OCR1A = iMax;
  OCR1B = iMax;
  
  // Forçar que o OC1B tenha fase oposta ao OC1A
  if (digitalRead(9)==digitalRead(10)){
    TCCR1C = 0b01000000;  // 64
    //utiliza o Force Output Compare FOC1B
  }

  // Reinicia os Timers
  GTCCR=0;
}

void expLevComandoO( float Dt ){
  unsigned long tInicio = micros();
  digitalWrite( pinoEnA, LOW );
  unsigned long tFinal = tInicio + Dt*1000UL;  
  while( micros()<tFinal ){}
  digitalWrite( pinoEnA, potenciaAtual );
  Serial.println( F("Comando concluido:") );
  Serial.print( "off " );
  Serial.println( Dt, 3 ); 
}

void expLevComandoC( long nRep, float DtRep, float Dt1 ){
  unsigned long tInicio;
  unsigned long tInicioCiclo=micros();
  for (long n=0; n<nRep; n++){
    tInicio = micros();
    digitalWrite( pinoEnA, LOW );
    unsigned long tFinal = tInicio + Dt1*1000UL;  
    while( micros()<tFinal ){}
    digitalWrite( pinoEnA, potenciaAtual );
    unsigned long tFinalCiclo = tInicioCiclo + DtRep*(n+1)*1000UL;  
    while( micros()<tFinalCiclo ){}
  }
  Serial.println( F("Comando concluido:") );
  Serial.print( "cycles " );
  Serial.print( nRep );
  Serial.print( ", " );
  Serial.print( DtRep, 3 ); 
  Serial.print( ", " );
  Serial.println( Dt1, 3 );
}

void expLevShowHelp(int tipo){
  if (tipo<=0){
    Serial.print( "iMax = " );
    Serial.print( iMax );
    Serial.print( " (freq.= " );
    Serial.print( 8000.0/(iMax+1) );
    Serial.print( "kHz)" );
    Serial.print( ", Pot. " );
    Serial.println( potenciaAtual? "On" : "Off" );
  }
  if (tipo>=0){
    Serial.println( F("Comandos validos:") );
    if (tipo==0){
      Serial.println( F("p$ - set pot $ (0 or 1)" ) );
      Serial.println( F("o# - off #ms" ) );
      Serial.println( F("c$,@,# - cyles: $ rep of o# in each @ms" ) );
      Serial.println( F("i$ - set iMax to $" ) );
      Serial.println( F("s - status (iMax, f(iMax), Pot)" ) );
      Serial.println( F("? - short Help (this help)" ) );
      Serial.println( F("h - full Help" ) );
    }else{
      Serial.println( F("p0 ou p1 -> Desliga (p0) ou liga (p1)") );
      Serial.println( F("      os emissores. Para exp. de queda") );
      Serial.println( F("o# -> Desliga os emissores por # ms.") );
      Serial.println( F("      Para exp. de oscilacoes amortecidas") );
      Serial.println( F("      (# um numero >0.0)") );
      Serial.println( F("c$,@,# -> faz $ repeticoes, a cada @ ms,") );
      Serial.println( F("      do comando o# (desligar por #ms") );
      Serial.println( F("      Para exp. de oscilacoes forcadas") );
      Serial.println( F("      ($ um numero inteiro >0)") );
      Serial.println( F("      (@ e # numeros >0.0, com @>#)") );
      Serial.println( F("i$ -> Define iMax em $ ciclos de clock") );
      Serial.println( F("      A frequencia sera 8MHz/(iMax+1)") );
      Serial.println( F("s   -> Mostra configuracoes (iMax, freq., Pot)" ) );
      Serial.println( F("?   -> Lembrete dos comandos" ) );
      Serial.println( F("h   -> Ajuda completa (esta ajuda)" ) );
    }
  }
}
