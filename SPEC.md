<br>
<br>
<br>
<br>
<br>
<br>
<br>
<br>

# <center> Not-Mustache </center> 
##### <center> Draft Specification </center>
##### <center> November 2025 </center>
##### <center> Tripp R. </center>

<br>
<br>
<br>
<br>
<br>
<br>
<br>
<br>
<br>
<br>
<br>
<br>

<hr>

##### Table of contents

##### [PREFACE](#preface)

[1.1 ............... Block Expressions](#block_expressions)  
[1.2 ........................... Comments](#comments)  
[1.3 ... Truthy / Falsy Expressions](#truthy_falsy)  
[1.4 ................. Standalone Lines](#standalones)  
[1.5 ....................... List Functions](#list_functions)  
[1.6 ....................... Variable Types](#variable_types)  


<hr>

##### Preface
Not-mustache is a near-identical derivative of the mustache templating templating language, the primary differences being additional list functions and the parsing of standalone lines.

<hr>


##### 1.1 -  Block Expressions <a id="block_expressions"></a>
<div>
    Block expressions are defined as an almost identical set of characters to standard mustache: 
</div>
<div>

`{{expression}}`

By default, it's evaluated contents will be HTML escaped.

Note that all variable names are <b>case-sensitive</b>.

</div>
<div>
If a block is preceded by the opening string `{{&` it's evaluated contents will not be HTML escaped.

`{{&non_escaped_expression}}`

</div>

<hr>

##### 1.2 - Comments <a id="comments"></a>
Comments in not-mustache are identical to mustache:
`{{!comment}}`
where any string preceded by a `!` will be ignored.

<hr>

##### 1.3 - Truthy / Falsy Expressions <a id="truthy_falsy"></a>
Expressions preceded with the block opening `{{#` will be considered to be truthy, whereas expressions preceded with the `{{^` will be treated as falsy.

An expression is truthy if:
###### Booleans: is true
###### Number: is non-zero
###### String: len > 0
###### List: element count > 0
###### Object: member count > 0

An expression should be considered falsy if its contents cannot be evaluated.

Truthy / Falsy Expressions are closed with the `{{/}}` block.
Optionally, the name of the variable being closed may be written within the closing block:
```
{{#var}}
{{/var}}
```



If the expression is truthy and the variable being checked in an array,
it should act as a for-each rather than an if condition, with
`{{.}}` refering to the current element of the array.


The `{{else}}` block is a special block which is executed when the preceding expression evaluates as falsy. 
It provides an alternative branch to the main {{#var}} or {{^var}} block that does not execute.


<hr>

##### 1.4 Standalone Lines <a id="standalone"></a>
A line is considered standalone if all of the contents on that line, other than any truthy/falsy not-mustache blocks, are whitespace. Standalone lines should be removed from the parsed output.

<hr>

##### 1.5 List Functions <a id="list_functions"></a>
The not-mustache 1.0 spec provides two list functions:
- `len(arr)`
- `arr[idx]`

The `len(arr)` evaluates to the number of variables within that list, and `arr[idx]` evaluates to the member of the list at a given index, where
index is a constant integer. If the idx is negative, the index should evaluate to `arr[len(arr)-|idx|]`.

<hr>

##### 1.6 Variable Types <a id="variable_types"></a>

`Boolean:` a type that can either be true or false. Evaluates to `true` or `false` <br>
`Number:` a type that holds a 64-bit floating point value. <br>
`String:` an array of bytes with a fixed length. <br>
`List:` an array of variables that can only be accessed by index notation `[i]`, not my name or structure. <br>
`Object:` a type that holds a list of variables that can be accessed by name with dot notation `.name` or index notation `[i]`. <br>
