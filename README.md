# Fundamentos de Sistema Embarcados (FSE) - Projeto 2

## Introdução
Esse projeto é uma implementação de um sistema que simula o controle de um Air Fryer digital.
O projeto envolve o desenvolvimeto das mecanicas básicas desse tipo de dispositivo, ligar/desligar, controlar temperatura (aplicando um PID por meio de uma ventoinha e um resistor), temporização e diferentes modos de alimentos pré configurados.


Para mais informações sobre o projeto acesse o Git Lab: [https://gitlab.com/fse_fga/trabalhos-2022_1/trabalho-2-2022-1](https://gitlab.com/fse_fga/trabalhos-2022_1/trabalho-2-2022-1)

## Instruções de execução

### Depêndencias
É necessário ter as seguintes depêndências, assim como os componendes eletrônicos que estão especificados no [Git Lab](https://gitlab.com/fse_fga/trabalhos-2022_1/trabalho-2-2022-1)

- WiringPi
- GCC
- make

### Instalação
- Clone o repositório e acesse a pasta
```bash
git clone https://github.com/lucasgbezerra/FSE_T2.git
cd FSE_T2/
```

- Em seguida realize a compilação da aplicação executando:
```bash
make all
```

- Após compilar, basta realizar o comando a seguir para executar a aplicação:
```bash
make run
```

- **OBS**: Caso a aplicação já tenha sido compilada, pode ser necessário utilizar o comando a segui:
```bash
make clean
```

## Utilização
- Para utilizar a aplicação é necessário acessar a Dashboard em um dos links a seguir:
    - [Dashboard Rasp 43](http://164.41.98.25:443/dashboard/657fba30-2706-11ed-be92-e3a443145aec?publicId=ba042a80-0322-11ed-9f25-414fbaf2b065)
    - [Dashboard Rasp 44](http://164.41.98.25:443/dashboard/a4b10e40-1d8b-11ed-a520-7b07ee36c1c6?publicId=ba042a80-0322-11ed-9f25-414fbaf2b065)

As dashboards são os painéis de controle da aplicação, apartir dela é possivel ligar/desligar a Air Fryer, Iniciar/Parar o uso, controlar a temperatura e o tempo para cozimento, e utilizar o menu para escolher um modo pré-configurado. Além de retornar visualmente as informações.

- **OBS 1**: Fisicamente é possível visualizar um display com informações sobre o que acontece com a AirFryer

- **OBS 2**: A cada utilização é gereda um arquivo *data.csv* contendo os dados (Data e hora, temperatura interna, temperatura externa, temperatura definida pelo usuário, valor de acionamento dos atuadores) em formato csv.

### Botões
- **Ligar/Desligar**: Ligar/Desligar a Airfryer (ao desligar um processo de resfriamento ocorre)
- **Iniciar/Parar**: Controla o funcionamento dos atuadores (ventoinha e resistor). É importante destacar que após iniciar o tempo pode não começar a a decrementar instântaneamente, o tempo só começa a contar a partir do momento que a temperatura desejada é alcançada
- **Tempo +/-**: Aumenta ou diminui o tempo de cozimento
- **Temperatura +/-**: Aumenta ou diminui a temperatura de cozimento
- **Menu**: Apresenta as opções de Tempo/Temperatura pré-configurados para alguns alimentos. Ao pressionar o botão o modo é alterado e refletido no display, caso queira outro modo basta apertar novamente, se estiver satisfeito com a opção pré-programada basta apertar em no botão iniciar;
Imagem do Servidor Central no seu MENU
- **CTRL-C**: Encerra todas as comunicações com periféricos e desligar os atuadores.

### Legenda display
- **TA**: Temperatura do Ambiente que é medida pelo sensor BME280.
- **TI**: Temperatura Interna da Air Fryer que é medida pelo Sensor DS18B20.
- **TR**: Temperatura de Referência definida pelo usuário.
- **Time**: Tempo definido pelo usuário.

## Testes Realizados
### Teste 1
![Teste 1 - Temperatura](https://raw.githubusercontent.com/lucasgbezerra/FSE_T2/main/assets/teste1_1.jpg?token=GHSAT0AAAAAABV7A53ACJ74F3XNSFSN3R7YYYVCUIA)
![Teste 1 - Sinal Controle](https://raw.githubusercontent.com/lucasgbezerra/FSE_T2/main/assets/teste1_2.jpg?token=GHSAT0AAAAAABV7A53APOJEJVWQMJXYSBIGYYVCVQA)
### Teste 2
![Teste 2 - Temperatura](https://raw.githubusercontent.com/lucasgbezerra/FSE_T2/main/assets/teste2_1.jpg?token=GHSAT0AAAAAABV7A53AUIAHGEW5KXIWLO4GYYVCV4A)
![Teste 1 - Sinal Controle](https://raw.githubusercontent.com/lucasgbezerra/FSE_T2/main/assets/teste2_2.jpg?token=GHSAT0AAAAAABV7A53ANCVME2A7YZTLQPXOYYVCV3A)
### Teste 3
![Teste 1 - Temperatura](https://raw.githubusercontent.com/lucasgbezerra/FSE_T2/main/assets/teste3_1.jpg?token=GHSAT0AAAAAABV7A53ARUWFRY262BAX5WNEYYVCWRA)
![Teste 1 - Sinal Controle](https://raw.githubusercontent.com/lucasgbezerra/FSE_T2/main/assets/teste3_2.jpg?token=GHSAT0AAAAAABV7A53AQH7GCUU7NBVHNVDCYYVCWTA)