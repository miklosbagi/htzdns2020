### HTZ Params Parser

function PARAMS_PARSER {
# rework params
for p in $@; do
    if [[ `$echo $p |$grep -E "^\-\-[a-z]+"` != "" ]]; then 
        n=`$echo "$p" |$sed 's#--##;s#^\(.\).*#\1#'`
	params="$params $n"
    fi

    if [[ `$echo $p |$grep -E "^-[a-z]+"` != "" ]]; then
       n=`$echo "$p" |$sed 's#-##;s#[a-z]#& #g;s# $##'`
       params="$params $n"
    fi
    if [[ `$echo $p |$grep -E "^[a-z]+"` != "" ]]; then 
       n=`$echo "$p" | sed 's#[a-z]#& #g;s# $##'`
       params="$params $n"
    fi
done

vrb=0
for p in $params; do
  case $p in
    -f | f | --force )				 force="1"; 	sw="$sw force";;
    -r | r | --rebuild )			 rebuild="1"; 	sw="$sw rebuild";;
    -t | t | --test )				 runtests="1"; 	sw="$sw test";;
    -c | c | --tests-coverage )                  coverage="1";	sw="$sw coverage";;
    -p | p | --pretend )			 pretend="1"; 	sw="$sw pretend";;
    -v | v | --verbose )			 verbose="1"; 	sw="$sw verbose"; (( vrb = vrb + 1 ));;
    -h | h | help | -help | /help| --help )      USAGE; 	exit;;
    * )                                          _WRN "Unknown argument provided (${@#})"; USAGE; exit 1;;
  esac
  shift
done
_VRB "Params: $sw"
}

### Usage manual next to params, so we can keep it updated.
function USAGE {
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
