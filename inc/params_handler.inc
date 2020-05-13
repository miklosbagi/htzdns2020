### HTZ Params Parser

function PARAMS_PARSER {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   # rework params
   for p in $@; do
       # capture --param and skim down to p
       if [[ `$echo $p |$grep -E "^\-\-[a-z]+"` != "" ]]; then 
          n=`$echo "$p" |$sed 's#--##;s#^\(.\).*#\1#'`; ps="$ps $n"
       fi
       # capture -prm and split up to p r m
       if [[ `$echo $p |$grep -E "^-[a-z]+"` != "" ]]; then
           n=`$echo "$p" |$sed 's#-##;s#[a-z]#& #g;s# $##'`; ps="$ps $n"
       fi
       # capture prm and split up to p r m
       if [[ `$echo $p |$grep -E "^[a-z]+"` != "" ]]; then 
          n=`$echo "$p" | sed 's#[a-z]#& #g;s# $##'`; ps="$ps $n"
       fi
   done

   vrb=0
   for p in $ps; do
       case $p in
       -f | f | --force )			force="1"; 	sw="$sw force";;
       -r | r | --rebuild )			rebuild="1"; 	sw="$sw rebuild";;
       -t | t | --test )			runtests="1"; 	sw="$sw test";;
       -c | c | --tests-coverage )              coverage="1";	sw="$sw coverage";;
       -p | p | --pretend )			pretend="1"; 	sw="$sw pretend";;
       -v | v | --verbose )			verbose="1"; 	sw="$sw verbose"; (( vrb = vrb + 1 ));;
       -h | h | help | -help | /help| --help )	USAGE; 		exit;;
       * )                                      _WRN "Unknown argument provided (${@#})"; USAGE; exit 1;;
     esac
     shift
   done
   _VRB1 "Params: $sw"
}

### Usage manual next to params, so we can keep it updated.
function USAGE {
   _VRB1 ">> $BASH_SOURCE :: ${FUNCNAME}"; _VRB2 "INPUT(S): (`for q in ${@#}; do $echo -ne $q\)\(; done`)"
   $echo
   $echo " 🌎 htzdns: A DNS record updater for dynamic IP addresses for domains managed at hetzner.de"
   $echo
   $echo " ℹ️  Usage params: [ -h | h | --help ]"
   $echo "                  [ -r | r | --rebuild ]"
   $echo "                  [ -pfvv | pfvv | --pretend --force --verbose --verbose ]"
   $echo "                  [ -t | t | --test ]"
   $echo
   $echo "   -f | f | --force	Force update to hetzner regardless of cache"
   $echo "   -r | r | --rebuild	Run configuration builder (interactive)"
   $echo "   -p | p | --pretend 	Do not make any changes, only show what would be done."
   $echo "   -v | v | --verbose  See what's happening."
   $echo "   -t | t | --test     Run tests (confirms compatibilty, warns on significant API changes)"
   $echo "   -h | h | --help	This usage manual."
   $echo
}
