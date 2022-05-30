# SalaMonitorada
Introdução
Basicamente a ideia do projeto consiste em um controle de uma sala/laboratório  que necessita de informações internas de controle de pessoas, sensor de temperatura      e umidade relativamente rígidos.
O monitoramento do local deve ser acessado via remoto por  meio do sistema de nuvem para projetos IOT “thingSpeak”


Usabilidade de cada módulo
O controle de pessoas autorizadas ou não do laboratório serão aplicados por meio do sensor RFID (MFRC-522).
O tempo de entrada/saída de funcionários e de aferição de temperatura e umidade serão monitorados por meio do software thingSpeak.
O monitoramento de temperatura e umidade serão feitos por meio do módulo (DHT11).
 O acesso internet será dado pela escolha do microcontrolador, ESP32.
Haverá um buzzer para aviso interno de instabilidade no laboratório.


Alocação de portas
Digitais: 4 (SCK, MISO, MOSI e SS do RFID) + 1 (BUZZER) = 5
Analógicas: 1 (DHT 22) 

![image](https://user-images.githubusercontent.com/57996705/171030034-69b4eb00-5b88-455c-b476-42b1a32c3844.png)
