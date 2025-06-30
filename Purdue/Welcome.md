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
    FCF->FCF_FinalYear;
    label = "*FCF*";
    fontsize = 20;
  }

  subgraph cluster_WACC {
    node [style=filled];
    FirstTerm -> SUM;
    SecondTerm -> SUM -> WACC;
    "E = Market value of equity" -> "V = Total capital (E + D)" [ label="DIVIDE" ];
    "V = Total capital (E + D)" -> "Cost of Equity" [ label="TIME" ];
    "Cost of Equity" -> FirstTerm
    "D = Market value of debt" -> "V = Total capital (E + D)" [ label="DIVIDE" ];
    "V = Total capital (E + D)" -> "Cost of Debt" [ label="TIME" ];
    "Cost of Debt" -> "1-Tax Rate" [ label="TIME" ];
    "1-Tax Rate" -> SecondTerm
    label = "*WACC*";
    fontsize = 20;
    color=blue
  }
  
   subgraph cluster_TerminalValue {
    node [style=filled];
    "terminal_growth_rate(g)" -> "(1+g)";
    "(1+g)" -> FCF_FinalYear [ label="X" ];
    FCF_FinalYear->Numer;
    WACC-> "terminal_growth_rate(g)" [ label="MINUS" ];
    "terminal_growth_rate(g)" -> Deno;
    Numer->TerminalValue
    Deno -> TerminalValue
    label = "*TerminalValue*";
    fontsize = 20;
    color=green
  }
  
    subgraph cluster_DCF {
    node [style=filled];
    FCF-> "of t"->Numer1;
    WACC->"sum1";
    "1+"->"sum1"->"raise to t"->Deno1;
    Numer1->"Summation(t..n)";
    Deno1->"Summation(t..n)";
    "Summation(t..n)" -> TerminalTerm [label="PLUS"];
    "1+"-> WACC -> "power of n"
    TerminalTermDeno
    TerminalTerm->PV [label="EQUAL"] ;
    label = "*DCF*";
    fontsize = 20;
    color=yellow
  }
  start -> a0;
  start -> b0;


  start [shape=Mdiamond];
  end [shape=Msquare];
}
