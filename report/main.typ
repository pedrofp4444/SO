#import "cover.typ": cover
#import "template.typ": *

#show: project

#cover(title: "Sistemas Operativos", authors: (
  (name: "Afonso Santos", number: "a104276"), 
  (name: "Hélder Gomes", number: "a104100"), 
  (name: "Pedro Pereira", number: "a104082")), 
  "11 de maio de 2024")

#set page(numbering: "i", number-align: center)
#counter(page).update(1)


// Make the page counter reset to 1
#set page(numbering: "1", number-align: center)
#counter(page).update(1)

= Introdução
#v(1em)
 
 Este relatório visa abordar o projeto da disciplina de Sistemas Operativos do ano letivo 2023/24, inserido no plano curricular da Licenciatura em Engenharia Informática, na Escola de Engenharia da Universidade do Minho.

O propósito central deste trabalho consiste no desenvolvimento de uma aplicação cliente/orquestrador. O projeto será dividido em duas partes fundamentais: o cliente, responsável por enviar solicitações de execução de comandos bash (Tasks) e consultas sobre programas em execução ao orquestrador; e o orquestrador, incumbido de armazenar e executar tarefas, bem como de manter o registo do estado dos programas concluídos.

De seguida, são apresentadas as decisões tomadas pelo grupo, com a devida justificação das mesmas, evidenciando as vantagens e desvantagens correspondentes. Segue-se uma explicação concisa da implementação adotada pelo grupo, com destaque para as partes relevantes para a compreensão da aplicação. Por fim, o relatório inclui uma análise de desempenho dos diferentes tipos de escalonamento utilizados, assim como sugestões para potenciais melhorias.
 
= Arquitetura
#v(1em)
Esta aplicação segue um modelo de client/orchestrator, pelo que existem dois programas o cliente e o orchestrator e uma componente extra que corresponde a um processo filho do processo do orchestrator. Na figura abaixo é apresentado de maneira abstrata. Como o orchestrator o centro principal da aplicação.

#figure(
  caption: [Esquema da arquitetura ],
 image("images/image.png", width: 110%)
)
== Cliente
#v(1em)
O programa é um processo independente do servidor por isso é possível vários clientes estarem ligados ao servidor em simultâneo e mandar vários pedidos, escrevendo-os num pipe com nome criado pelo orchestrator. O cliente recebe os argumentos processa para o formato utilizado pelo o orchestrator, e cria um canal de comunicação, um pipe com nome (FIFO's) para qual o orchestrator imprime o id correspondendo ao pedido enviado posteriormente. Se for enviado um pedido de status, a resposta é enviada através desses FIFOs." 
 
== Orchestrator
#v(1em)
O Orchestrator é o principal componente da aplicação, responsável por receber todos os pedidos do cliente e colocá-los em uma fila de espera para execução. Ele pode receber simultaneamente vários pedidos de diferentes clientes, graças à presença de um processo filho que responde aos pedidos com um identificador único de tarefa e os encaminha para o processo pai.

Dependendo do tipo de tarefa, o Orchestrator pode inseri-la na fila de espera ou atualizar as informações existentes. Quando a fila de espera não está vazia, o próximo pedido a ser executado é retirado, e essa seleção varia dependendo do algoritmo de escalonamento fornecido ao Orchestrator durante a inicialização, podendo ser FCFS (First-Come, First-Served) ou SJF (Shortest Job First) com prioridade para evitar a estagnação.

Após a seleção, o Orchestrator verifica se há possibilidade de criar um novo processo filho para executar a tarefa de forma concorrente. Se for possível, o Orchestrator é notificado e o processo filho é criado e as instruções são executadas, redirecionando a saída padrão e os erros para um arquivo com o identificador da tarefa. Em seguida, o processo filho é fechado e os identificadores das tarefas, juntamente com seus tempos de execução, são registados em um arquivo e suas informações são atualizadas.

Se não for possível executar a tarefa, todas as outras funcionalidades do Orchestrator continuam operando, exceto pela criação de novos processos filhos para execução das tarefas, que só voltarão a estar disponíveis quando um dos processos filhos terminar sua execução.

= Implementação
#v(1em)
Serão apresentados de seguida os detalhes de implementação considerandos relevantes para a compreensão do projeto. 

Adotamos a filosofia  _simple and beautiful _  para tornar o programa mais simples e genérico possível, isto é, tentar que este não seja limitado apenas às restrições do enunciado (nomeadamente no número e nome dos programas). Além disso, procuramos desenvolver uma aplicação com um grau de escalonamento relativamente alto, ou seja que consiga suportar um elevado número de pedidos simultaneamente.
  
== Tarefa
#v(1em)
Para uniformizar a implementação de tarefas entre servidor e cliente, e de forma a facilitar o processamento do pedido no lado do servidor foi implementada a estrutura que armazena a informação de uma tarefa (Task).A definição da estrutura é a seguinte
#let cod = {
```c
typedef struct {
  Type type; 
    // Identifica o tipo da tarefa
  Phase phase;
    // Identifica a fase que a tarefa se encontra 
  char program[308]; 
   // Linha de comando bash para executar
  int duration; 
  // Tempo estimado, em ms, para a execução da tarefa 
  
  int id;
   // Identificador único da tarefa
  pid_t pid;
   // PID para comunicar com o cliente
  
   struct timeval start_time;
  
} Task;
```
}

#let code ={
  ```c
  typedef enum type {
    STATUS, 
      // É uma tarefa tipo status
    EXECUTE 
      // É uma tarefa tipo execute  
  } Type;

  ```
}

#let text={
  block(
  fill: luma(230),
  inset: 8pt,
  radius: 4pt,
  code
  
)
v(1em)
set align(left)
  [
    O parâmetro 'id' e a instância 'start_time' são inicializados no Orchestrator assim que a tarefa chega, pois isso nos permite calcular a diferença entre esse momento e o momento em que a tarefa é concluída. Isso fornece uma medida precisa do tempo que a tarefa permaneceu no Orchestrator.

    O parâmetro 'phase' será explicado posteriormente no relatório 
  ]
}




#grid(
  columns: 2,
  column-gutter: 30pt,
  text,

  block(
    fill: luma(230),
    inset: 8pt,
    radius: 4pt,
    cod
  )

)

== Gestor de tarefas
#v(1em)
Como referido anteriormente quando uma tarefa chega ao orchestrator a mesma é atribuida um id e é inserida numa fila de espera, ou seja, numa estrutura de dados, como nós queremos uma solução simples e com um alto grau de escalonamento pensamos primeiramente implementar uma queue e uma queue com prioridade para o tipo de escalonamento FCFS e o tipo de escalonamento SJF respetivamente. Mas com duas estruturas de dados semelhantes decidimos torna-las numa queue genérica mudando só a abordagem de remoção da tarefa da mesma.

Quando o escalonamento é definido como _First Come First Serve_, o 1º elemento que entrou na queue será o primeiro a ser removido. E quando o escalonamento é definido como _Shortest Job First_ com prioridade a tarefa com menor tempo, que consiste em remover o elemento com menor tempo presente na queue e para evitar a estagnação decidimos que sempre removemos um elemento retiramos 10 unidades de tempo a todas as tarefas (tasks.duration)  presente na lista de espera, assim as tarefas presentes vão progressivamente subindo na hierarquia.

Umas das ideias foi implementar hashtables e miniheaps mas são estruturas de dados complexas que requerem uma implementação cuidadosa e detalhada. Para muitos projetos, especialmente aqueles que visam a simplicidade e a eficiência, a utilização dessas estruturas pode introduzir uma complexidade desnecessária no código.

As hash tables, por exemplo, envolvem lidar com colisões, gestão de carga e dispersão de chaves, o que pode tornar o código mais difícil de entender e manter. Já as miniheaps, embora sejam uma versão simplificada das heaps tradicionais, ainda requerem uma lógica de manutenção de ordem e estruturação dos elementos que pode ser complexa de implementar corretamente.

== Pedido status

Quando um cliente faz um pedido de status, espera-se uma resposta rápida do Orchestrator, que deve ser capaz de fornecer informações atualizadas sobre o estado das tarefas. Para atender a essa necessidade, enfrentamos duas questões cruciais: a primeira é garantir que todas as tarefas sejam registadas e a segunda é classifica-las imediatamente de acordo com seu estado.

Para resolver essas questões, decidimos associar o tipo _phase_ a cada uma das tarefas, proveniente de uma estrutura enum que indica as várias fases das tarefas: na fila de espera (SCHEDULED), em execução (EXECUTING), tarefa completa (COMPLETED) e quando a tarefa está a ser criada (NONE). Todas as tarefas são armazenadas numa estrutura Status.



#v(1em)
#let Metrics={
```c
typedef struct {
  Task metrics[MAX_TASKS]
   /* Arrays onde são guardadas 
   informações das tarefas */
  int end
   // o fim do array 
} Status 
```
}
#let Phase ={
  ```c
typedef enum phase{
  SCHEDULED, 
      // tarefa escalonada
  EXECUTING,
      // tarefa a executar
  COMPLETED, 
      // Tarefa realizada
  NONE
      // Tarefa Inicializda
}Phase;

```
}


#let enums={
  grid(
    row-gutter: 23pt,
    rows:  2,

    block(
    fill: luma(230),
    inset: 8pt,
    radius: 4pt,
      Metrics
    ),

     block(
    fill: luma(230),
    inset: 8pt,
    radius: 4pt,
       
       Phase
    )
   
  )
}

#grid(
  columns: 2,
  column-gutter: 30pt,
  enums,

  block(
    fill: luma(230),
    inset: 8pt,
    radius: 4pt,
    cod
  )
)


#pagebreak()

Antes do Orchestrator inserir a tarefa na fila de espera e nos status, muda a fase da mesma para escalonada. Antes da mesma ser executada, o seu estado é mudado para o respetivo e o orchestrator  é notificado para poder atualizar as informações da tarefa no array Status. Quando a mesma termina de ser executada, a sua fase é mudada e é reenviada ao servidor para poder atualizar as informações e informar que outra tarefa pode ser executada.

Quando um pedido de status é recebido pelo Orchestrator, ele pode acessar facilmente as informações contidas na estrutura Status. Em seguida, para garantir uma resposta rápida ao cliente, o Orchestrator cria um processo filho dedicado a encaminhar os status diretamente ao cliente. Isso permite que o cliente receba as informações solicitadas de forma eficiente.



= FCFS _vs_ SJF

First Come First Serve (FCFS) e Shortest Job First (SJF) são dois algoritmos de escalonamento amplamente conhecidos, cuja aplicação vai além dos sistemas operativos, sendo também relevantes em muitos aspetos do nosso dia a dia. Decidimos implementar e comparar esses dois métodos para entender melhor como eles se comportam em diferentes cenários.

Para testar a eficácia da nossa aplicação, desenvolvemos scripts bash que simulavam a geração de um número variável de tarefas. Inicialmente, ao executar os testes com um número reduzido de tarefas, notamos uma certa diferença de desempenho entre os dois algoritmos. O FCFS parecia mais vantajoso em termos de tempo de resposta em comparação com o SJF.

Entretanto, à medida que aumentávamos a carga de trabalho, introduzindo um maior número de tarefas e incluindo algumas que demandavam mais tempo para serem concluídas, essa diferença diminuía consideravelmente. O SJF, com sua abordagem de priorizar as tarefas mais curtas, mostrou-se mais eficiente em situações em que tarefas mais demoradas estavam presentes, minimizando o tempo médio de espera e melhorando o desempenho geral do sistema.

Portanto, os nossos resultados indicam que a escolha entre FCFS e SJF depende significativamente da natureza das tarefas e da carga de trabalho esperada. Enquanto o FCFS pode ser preferível para aplicações com interações simples e tempos de execução uniformes, o SJF se destaca em cenários onde a variabilidade no tempo de execução das tarefas é significativa ou quando lidamos com grandes volumes de trabalho.

Utilizando os programas para testes fornecidos pelos docentes, decidimos apresentar a diferença anteriormente apresentada, através de um gráfico de barras.

#image("images/FCFS e SJF.png")


= Conclusão

Deste modo,  e para concluir, o grupo considera ter cumprido todos os requisitos pretendidos, implementando uma aplicação cliente/servidor através de comunicação entre processos não só fiável mas também com um bom desempenho, permitindo aplicar transformações a ficheiros de forma concorrente.

Neste projeto existem aspetos que podiam ter sido melhorados. Um desses aspetos seriam a sua modularização, a utilização de sinais para uma possível implementação do robin bound e melhorar o desempenho do servidor ao permitir que várias conexões sejam processadas em paralelo, como por exemplo, um processo que só adiciona e remove as tarefas da fila de espera.

Em suma, o grupo considera o seu projeto como bem sucedido, tendo em conta que conseguimos desenvolver na prática os conhecimentos obtidos da unidade curricular de sistemas operativos e consolidar o nosso conhecimento de C.   
