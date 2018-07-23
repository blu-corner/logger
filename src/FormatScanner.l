%option prefix="inscribe_"

%{
#include "tokens.h"
%}

%%

"\{time\}"      {
                    return TIME;
                }

"\{severity\}"  {
                    return SEVERITY;
                }

"\{name\}"      {
                    return NAME;
                }

"\{message\}"   {
                    return MESSAGE;
                }

.               {
                    ECHO;
                }

%%
