class FormulationPass : StateMentPass{
    StateMent * nop_;
    std::map<Varience*,Varience*> arg2var_map_;
    std::map<Varience*, Varience *> varP_varVP_map_;

    std::map<Varience*, Varience *> varP_varPV_map_;

    FormulationPass() {
        nop_ = Nop::make();
    }
virtual    StateMent* pass_(Block * stat ) ;
virtual    StateMent* pass_(LetStat * stat ) ;
virtual    StateMent* pass_(Gather * stat   ) ;
virtual    StateMent* pass_(Load * stat);
virtual    StateMent* pass_(Add * stat   );
virtual    StateMent* pass_(Div * stat   ) ;
virtual    StateMent* pass_(Store * stat  ) ;
virtual    StateMent* pass_(Scatter * stat  ) ;
}
