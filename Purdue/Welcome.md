# http://www.graphviz.org/content/cluster

digraph G {
  graph [fontname = "Handlee"];
  node [fontname = "Handlee"];
  edge [fontname = "Handlee"];

  bgcolor=transparent;

  subgraph cluster_FCF {
    style=filled;
    color=lightgrey;
    node [style=filled,color=pink];
    OperatingCashFlow -> CapEx [ label="MINUS" ];
    CapEx -> FCF;
    NetIncome -> Depreciation [ label="PLUS" ];
    Depreciation -> "Delta(WorkingCapital)" [ label="MINUS" ];
    "Delta(WorkingCapital)" -> CapEx [ label="MINUS" ];
    label = "*FCF*";
    fontsize = 20;
  }

  subgraph cluster_1 {
    node [style=filled];
    b0 -> b1 -> b2 -> b3;
    label = "*process #2*";
    fontsize = 20;
    color=blue
  }
  start -> a0;
  start -> b0;
  a1 -> b3;
  b2 -> a3;
  a3 -> a0;
  a3 -> end;
  b3 -> end;

  start [shape=Mdiamond];
  end [shape=Msquare];
}
