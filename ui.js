var editor;

$(document).ready(function(){

    function addClass(element, className) {
	if (!editor.win.hasClass(element, className)) {
	    element.className = ((element.className.split(" ")).concat([className])).join(" ");
	}
    }

    function removeClass(element, className) {
	if (editor.win.hasClass(element, className)) {
	    var classes = element.className.split(" ");
	    for (var i = classes.length - 1 ; i >= 0; i--) {
		if (classes[i] === className) {
		    classes.splice(i, 1);
		}
	    }
	    element.className = classes.join(" ");
	}
    }

    var textarea = document.getElementById('code');
    editor = new CodeMirror(CodeMirror.replace(textarea), {
	height: "350px",
	content: textarea.value,
	path: "CodeMirror-0.91/js/",
	parserfile: ["../contrib/scheme/js/tokenizescheme.js",
    "../contrib/scheme/js/parsescheme.js"],
	stylesheet:  "CodeMirror-0.91/contrib/scheme/css/schemecolors.css",
	autoMatchParens: true,
	lineNumbers: true,
	disableSpellcheck: true,
	markParen: function(span, good) {addClass(span, good ? "good-matching-paren" : "bad-matching-paren");},
	unmarkParen: function(span) {removeClass(span, "good-matching-paren"); removeClass(span, "bad-matching-paren");}
    });
 });

function execute(str){
    
}