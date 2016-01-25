%%{

machine en_assimilation;

include apostrophe "symbol.rl";

en_assimilation = apostrophe 
                  ("tis"i | "twas"i | "twill"i | "twere"i | "twould"i)
                  ;

}%%
