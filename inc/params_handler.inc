### HTZ Params Parser

function PARAMS_PARSER {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   # rework params
   for p in $@; do
       # check for test API key
       if [[ `$printf -- "$p" | $grep -o -w '\w\{32,32\}'` != "" ]]; then test_api="$p"; continue; fi
       # capture --param and skim down to p
       if [[ `$printf -- "$p" |$grep -E "^\-\-[a-z]+"` != "" ]]; then
          n=`$printf -- "$p" |$sed 's#--##;s#^\(.\).*#\1#'`; ps="$ps $n"
       fi
       # capture -prm and split up to p r m
       if [[ `$printf -- "$p" |$grep -E "^-[a-z]+"` != "" ]]; then
           n=`$printf -- "$p" |$sed 's#-##;s#[a-z]#& #g;s# $##'`; ps="$ps $n"
       fi
       # capture prm and split up to p r m
       if [[ `$printf -- "$p" |$grep -E "^[a-z]+"` != "" ]]; then
          n=`$printf -- "$p" | sed 's#[a-z]#& #g;s# $##'`; ps="$ps $n"
       fi
   done
   echo $test_api
   vrb=0
   for p in $ps; do
       case $p in
       -f | f | --force )			force="1"; 	  sw="$sw force";;
       -r | r | --rebuild )		rebuild="1"; 	sw="$sw rebuild";;
       -t | t | --test )			runtests="1"; sw="$sw test";;
       -p | p | --pretend )		pretend="1"; 	sw="$sw pretend";;
       -v | v | --verbose )		verbose="1"; 	sw="$sw verbose"; (( vrb = vrb + 1 ));;
       -h | h | --help | /? )	USAGE;        exit;;
       # Unexposed args
       -c | c )               coverage="1";	sw="$sw coverage";;
       * )                    _WRN "Unknown argument provided: ${@#}"; USAGE; exit 1;;
     esac
     shift
   done
   _VRB1 "Params: $sw"
}

### Usage manual next to params, so we can keep it updated.
function USAGE {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $printf -- $q\)\(; done`)"
   _SAY "\n 🌎 htzdns: A DNS record updater for dynamic IP addresses for domains managed at hetzner.de\n"
   _SAY " ℹ️  Usage params: [ -h | h | --help ]"
   _SAY "                  [ -pfrvv | pfrvv | --pretend --force --rebuild --verbose --verbose ]"
   _SAY "                  [ -t | t | --test ]\n"
   _SAY "   -f | f | --force	Force update to hetzner regardless of cache"
   _SAY "   -r | r | --rebuild	Run configuration builder (interactive)"
   _SAY "   -p | p | --pretend	Do not make any changes, only show what would be done."
   _SAY "   -v | v | --verbose	See what is happening."
   _SAY "   -t | t | --test	Run tests (confirms compat, cries out on API changes)"
   _SAY "   -h | h | --help	This usage manual.\n"
   _SAY " Certain arguments can be used in combination with each other."
   _SAY " The following scenario would require you to add multiple arguments:"
   _SAY " - You don't want to write any changes (pretend)"
   _SAY " - You want to see what would be updated regardless of what is cached"
   _SAY "   or data in hetzner records (force)"
   _SAY " - You want to build a new configuration to add more records (rebuild)"
   _SAY " Then you can execute with --pretend --force --rebuild (or -prf for short)\n"
   _SAY " # ./htzdns.sh -prf\n"
   _SAY " Adding verbosity or double verbosity to this mix is also possible as -prfvv.\n"
}
