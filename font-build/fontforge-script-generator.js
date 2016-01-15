var script = "SelectAll()\n"

var zhen = ["0123456789一二三四五六七八九十电池月日号年星期天早上好晚安下午",
"0123456789&'()*+,-.AdcYRHNXQTZShWaxw"]

if(zhen[0].length!=zhen[1].length)throw "diff len"

script += _.map(zhen[0], function(c){return 'SelectFewer(0u'+c.charCodeAt(0).toString(16)+')'}).join("\n")

script+="\nDetachAndRemoveGlyphs()\nSelectNone()\n"

script+= _.compact(_.times(zhen[0].length, function(i){
	if(zhen[0].charCodeAt(i) == zhen[1].charCodeAt(i)) return ""
return "SelectNone()\nSelect(0u" + zhen[0].charCodeAt(i).toString(16) + ")\nSetUnicodeValue(0u" + zhen[1].charCodeAt(i).toString(16) + ")"
})).join('\n')

script += '\nReencode("latin1")\nReencode("compacted")'

console.log(script)