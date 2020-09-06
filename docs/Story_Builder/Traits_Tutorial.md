# Tutorial: Traits System

One of the essential features of the SkotOS architecture is the extensive use of object properties (aka the Property System). And of all the different uses for properties, the `trait:*` properties are among the most powerful and accessible, as well as being a solid place for the new StoryBuilder to learn about how properties work in SkotOS.

## A simple belt

First `+spown` for yourself one of our common Shared System objects, a simple belt.

```
> +spawn Shared:clothing:ageless:accessories:belt-simple

[1 -> Daris]
[Item was proofed Fri Nov 17 02:07:08 2006 by Christophera.]
```

An important point to note first is that this Shared object, so it has been named very carefully.

*  `Shared:` - is an object that is shared in common across multiple Chat Theatres.

*  `:clothing:` - is a clothing object that is intended to be worn.

*  `:ageless:` - that this clothing is not from any particular period of time or genre. It is timeless from the distant past to the distant future.

*  `:accessories:` - that this clothing is not a regular item of clothing, but instead it "accessorizes" other clothing by being on top.

*  `:belt-` - that this particular accessory is a belt.

*  `-simple` - and finally of all belts, this object is specifically a "simple" belt.

All `Shared:clothing:` object should work with the `+traits` command. Let's try it:

```
+ traits my belt

(popup window)

[Shared:clothing:ageless:accessories:belt-simple]#17856

Appearance (brief, look, examine):

Daris' white cotton belt
A belt made of white cotton.
A white cotton belt. It's a strip of flexible material which is worn snugly about the waist.

Attributes:
 white (color)
 (condition)
 (fit)
 cotton (material)
 (symbol)
```

You now see the three key default "prime" descriptions of this simple belt. (see Description System).

*  `brief description` - the shortest text that is used in composite descriptions, i.e. `look at Daris` or as the result of actions `drop my belt`. My own convention for briefs is list what do might notice from across the room. So in this case, the brief is just a `white cotton belt`.

* `look description` - this is what someone notices when they specifically look at the object, e.g. `look at my belt` or `look at Daris' belt`. Again, my own convention is that this is what you'd notice if you saw the item, but not for so much detail or for so long  that anyone else might notice that you are looking.

* `examine description` - this is what you'd get if you looking carefully or closely. In this case, others will likely notice that you are looking. You don't want to necessarily include every possible detail, but our convention is that every possible additional detail (see Detail System) should be eventually discoverable from something in an examine of the "prime" detail. So no hidden details that require you do a noun hunt to find it!

You also note that this spown of the simple belt currently only has two `traits:` defined: a `trait:color` of white and `trait:material` of cotton.

By convention, all `Shared:clothing:` must have least have these two traits defined. Our assumption is that these both color and material are obvious even from across the room. Most `Shared:clothing:` objects default to white and cotton, but there are obvious exceptions.

Note however, that these two default traits are not the only ones possible. Try setting `trait:condition` to `old` or `clean`, or the fit to `tight` or `loose`, or add a `rose` symbol to the belt. Press the `<tab>` key to see the 3 descriptions change to reflect your changes to the traits. 

```
[Shared:clothing:ageless:accessories:belt-simple]#17856

Appearance (brief, look, examine):

Daris' white cotton belt
An old belt made of white cotton, embroidered with a rose.
A white cotton belt, fitted in a loose manner. It's a strip of flexible material which is worn snugly about the waist. It is embroidered with a rose. It is old.

Attributes:
 white (color)
 old (condition)
 loose (fit)
 cotton (material)
 rose (symbol)
```

Also, note that if you change the condition trait from `old` to `new` note that the look description uses the proper article for a vowel vs a consonant.

These traits might also be visible in additional details of this belt, such as buckles, etc. But of course, this is a simple belt, so it has no details beyond that of the "prime" detail.

## SIDEBAR: Use only single word traits!

You should not use multiple words in a trait, as the word in the trait is also used as adjective. You want `get white belt` to not succeed if there are only black belts in the room. You can use hyphenated words as traits, but in general you want to try to avoid them if you can.

If an object needs a multi-word adjective in a trait, consider use a new single word like "ancient" instead of "very old". Alternatively, you can add an additional trait property to the object and all the descriptions and details.  However, that can be quite time consuming!

Instead, if you need to add a description phrase that needs to be two or more words, you might consider making clone (not spawn) of the original Shared object, rename it accordingly (e.g. `Shared:clothing:ageless:accessories:belt-with-spider-silk-embroidery`, then add that phrase added as just text rather than as as a new trait.

## When +traits is not available, +stats & +setp

Many objects use traits to modify descriptions, but not all objects work with +traits (mostly `Shared:clothing:` so far.) So how do you modify them?

First let's see what `traits:` have been set on my belt:

```
+stat my belt "property:trait:*"

-- Properties (trait:*)--
Property: trait:color = "white"
Property: trait:condition = "old"
Property: trait:fit = "loose"
Property: trait:material = "cotton"
Property: trait:symbol = "rose"
```

But how do I know what other traits are possible? There is a special property that may give you a clue, but note that not all objects have this property (yet):

```
+stat my belt "property:traits:adj-map

-- Properties (traits:adj-map)--
Property: traits:adj-map = ([ "color":({ "default" }), "condition":({ "default" }), "fit":({ "default" }), "material":({ "default" }), "symbol":({ "default" }) ])
```

To set a trait, we use the `+setp` command:

```
> +setp my belt "trait:color black"

Setting 'trait:color' in <[Shared:clothing:ageless:accessories:belt-simple]#17856> to "black".

> look at my belt

You look at your black cotton belt.
 An old belt made of black cotton, embroidered with a rose. 
```

## Using traits in Detail Descriptions


Now lets look at this parent of this spawned object in Woe view:

```
> +obname my belt

It is a spawn -- [Shared:clothing:ageless:accessories:belt-simple]#17856 ; "default" [V] [M]
-- of Shared:clothing:ageless:accessories:belt-simple [V] [M] [O]
-- of Shared:Local:clothing:UrClothing [V] [M] [O]
```

Click on [V] of the parent to see the origin of the details and descriptions of the spawned object (or type `+tool woe view %Shared:clothing:ageless:accessories:belt-simple` which does the same thing).

If you click [E] on the "prime" `Base:Detail` you'll see a number of adjectives and descriptions: 

Adjectives:
* flexible
* simple

Note that none of these adjectives are traits. When trait values are set on an object, the adjectives will be added by the system. So you only need to add adjectives here that are used in the "prime" detail that are not traits.


## The Prime Descriptions

Brief:
```
$(this.trait:color) $(this.trait:material) belt
```

Look:
```
<describe-props which="trait:condition trait:style" noun="simple" cap="1"/> belt made of $(this.trait:color) $(this.trait:material){? | $(this.trait:symbol) |, embroidered with <describe-prop what="trait:symbol"/>|}.
```

Examine:
```
<describe-props which="trait:color trait:material trait:style" cap="1"/> belt{? | $(this.trait:fit) |, fitted in <describe-prop what="trait:fit"/> manner|}. It's a simple strip of flexible material which is worn snugly about the waist.{? | $(this.trait:symbol) | It is embroidered with <describe-prop what="trait:symbol"/>.|}{? | $(this.trait:condition) | It is $(this.trait:condition).|}
```

## Generic Objects

You'll note that there is a complex text markup in the text these descriptions. Instead you could, in theory, just do:

Adjectives:
```
white
simple
cotton
```

Brief:
```
white belt
```

Look:
```
A simple belt made of white cotton.
```

Examine:
```
A white cotton belt. It's a simple strip of flexible material which is worn snugly about the waist.
```

And then for each variant create another object and just change those descriptions.

This is what we call a "Generic" object. These can still be very useful for a one-time prop, but they have no flexibility or opportunties for re-use.

# SAM in Descriptions

The complex markup is called SAM which stands for "Skotos Active Markup". Is is related to XML and HTML markup, but is intended specially for this type of function: to actively changed depending on the situation.

Like HTML, SAM consists of a number of tags.

The simplest SAM tag is to just output the value of a property as text:

```
$(this.trait:color)
```
You will most often see this kind of tag in Brief descriptions. 

So in this case the `trait:color` of the `this.` object is output, i.e. "white". If the trait doesn't exist, nothing will be output. so you could in theory have as a brief `$(trait:condition) $(trait:color) 
(trait:material) $(trait:fit) $(trait:symbol) belt` but as we said above, Brief descriptions should be simple, and as we said ealier, the traits of `trait:color` and `trait:material` should exist by convention, so we normally just use those.

A problem with this kind of tag is what to do about form of article required when it is used in a sentence, which is required for Look and Examine descriptions.

```
<describe-prop what="trait:color"/> robe.
```

This will output "a white robe." or "an orange robe." as appropriate to the value of the trait.

But what if you want that at the beginning of a sentence?

`<describe-prop what="trait:color" cap="1"/> robe.`

This will output "A white robe." or "An orange robe." as appropriate.

Sometimes you want to output multiple traits, but sometimes those traits may be empty, so what do you capitalize?

`<describe-props which="trait:color trait:material" cap="1"/> robe.`

This will do the right thing, "A white cotton robe." or "An orange organza robe.".

However, a complication can be if the the trait is blank. How do we use the correct article then?

`<describe-prop what="trait:condition noun="simple" cap="1"/> robe.`

As `trait:condition` is optional, you'll still get the correct output: "A simple robe." or "An old simple robe."

## Conditional SAM

The most complex of these common SAM tags is the conditional:

```
{? | $(this.trait:symbol) | It is embroidered with <describe-prop what="trait:symbol"/>.|}
```

This will only output text if `trait:symbol` is has some value. But if it exists, it will output "It is embroidered with a rose."

However, in some cases you might want the opposite. Instead, there can be output when a trait is empty:

```
{? | $(this.trait:symbol) | | It is blank and not embroidered. }
```

And you can have both:

```
{? | $(this.trait:symbol) | It is embroidered with <describe-prop what="trait:symbol"/>.| It is blank and not embroidered. }
```

Conditional SAM can be combined in quite complex ways. Take a look at the Examine description of this simple belt for an example.

## Other SAM Tags

The above tags are the ones that are used the most often in desciptions, but there are large number of others that can be useful.


### OneOf

A powerful but very simple SAM tag is the OneOf -- basically using a OneOf in your text allows you to randomize the output.

```
   Look: [
      You are inside {a large|an oversize|a} tent.
   ]
   Output(s):
      > look at tent
      You are inside a large tent.
      > look at tent
      You are inside an oversize tent
      > look at tent
      You are inside a tent.
      >
```

### Output a Property

You can also output the value of a property very easily.

```
   Look: [
      The gem is $(this.trait:gemcolor).
   ]
   Output(s):
      > look at gem
      The gem is blue.
      > +setp gem "trait:gemcolor green
      Setting 'trait:gemcolor' in <[Examples:complete:sam:gem]#4544> to "green".
      > look at gem
      The gem is green.
      >
```

In the above example, $(this) refers to the object itself, the gem itself. $(this.trait:gemcolor) refers to the value of the property "trait:gemcolor" inside the gem.


### Other Arguments

$(this) is only one of many 'arguments' that SAM knows about that is different depending on the circumstance that the SAM is executed.

In almost every situation, the following arguments are available:

   * *`$(actor)`*  -- The object executing the action, e.g. whomever is taking the sword  
   * *`$(action)`*  -- The name of the action currently executing, e.g. take
   * *`$(this)`* -- The object containing the SAM itself, e.g. the sword
   * *`$(target)`* -- The detail of the object that is the target of the action.
   
In addition, in many situations, the following additional arguments are available:

   * *`$(verb)`* -- The verb used in the action, e.g. 'polish' in "polish the sword with my cloth"
   * *`$(dob)`* -- The object that is the direct object of the action, e.g. the sword
   * *`$(using)`* -- the object that is the indirect object of the action, e.g. the cloth
   
### References

In addition to arguments, you can also reference specific objects.

   * $(Full:Woe:Name) -- the object with the woe name "Full:Woe:Name"
   * $[${Full:Woe:Name}] -- the object that is currently named "Full:Woe:Name", and if the object is renamed, the reference will change with it.
   
 
### Property References

Once you have object defined, either by argument $this or by reference $(Full:Woe:Name) or $[${Full:Woe:Name}], you can directly output the value of properties that are in these objects into your descriptions, just by adding a period followed by the property name:

   * *`$(this.trait:variant)`*
   * *`$(actor.trait:variant)`*
   * *`$(Full:Woe:Name.trait:variant)`*
   * *`$[${Full:Woe:Name.trait:variant}]`*

Some commonly used properties that output text:
   * *`$(this.base:possessive)`* -- possessive pronoun e.g. 'his'  
   * *`$(this.base:objective)`* -- objective pronoun e.g. 'him'  
   * *`$(this.base:pronoun)`*  -- basic pronoun e.g. 'he'  
   * *`$(this.base:light-category)`*  -- object's luminosity, e.g. "dark", "dim", or "bright" 
   * *`$(this.details:default:description:brief)`* -- object's default brief description

Examples of use:

```
   Look: [
      $(this.base:pronoun) looks very fierce. A scar runs from $(this.base:possessive) cheek to $(this.base:possessive) chin, which doesn't seem to bother $(this.base:objective).
   ]
   Output:
      > look at warrior
      She looks very fierce. A scar runs from her cheek to her chin, which doesn't seem to bother her.
      >
```

```
   Look: [
      You are in a $(this.base:light-category) room.
   ]
   Output:
      > look
      You are in a dim room.
      >
```

Some property references don't output text, but instead refer to other objects:
   * *`$(this.base:environment)`* -- object's environment  
   * *`$(this.base:worn-by)`* -- object's wearer (same as wielded-by)  
   * *`$(this.base:wielded-by`* -- object's wielder (same as wielded-by)  

These are not that useful to use directly, but in turn you can then use properties with these:

   * *`$(this.base:environment.base:possessive)`* -- object's environment possessive pronoun
   * *`$(this.base:worn-by.base:pronoun)`* -- object's wearer's basic pronoun

Details of objects can be output:

  * *`$("this.details:default:descriptions:look")`* -- the prime detail's look description
  * *`$("this.details:lamp:descriptions:brief")`* -- the lamp detail's brief description

Some detail IDs have spaces in them. The way to refer to a SAM reference with a space in it is to put it in quotes -- e.g.

   * *`$("this.details:exit west:descriptions:look")`* -- the look description of exit west

rather than

   * *`$(room.details:exit west:descriptions:look)`* -- won't work

### XHTML Tags

There are some XHTML formatting tags that can be useful are are available in SAM

   * *`<br/>`* -- a line break, a single newline
   * *`<sbr>`* -- single break, inserts <br/> when used in a webpage, or a newline when used elsewhere.
   * *`<p/>`* -- a paragraph break, two newlines
   * *`<pre>text</pre>`* -- will output the text in a monospace font, without formatting
   

### Common Sam Tags

There are various times when you need specific output that is not easily available in a property, or the property is in the wrong form. These SAM tags can be useful in these cases:

These are SAM Tags for describing things:

   * *`<describe what="$(this)"/>`* -- This will output the brief of $(this) object, e.g. "a sword"
   * *`<describemany what="$(this.base:inventory)"/>`* -- Describe many will output multiple briefs, as if in an inventory, of the objects in $(this.base:inventory).
   * *`<describe-holder what="$(this)"/>`* -- Equivalent to `<describe what="$(foo.base:holder)"/>'s` except when the person's name ends in 's' or 'x' in which case it does the right thing and just adds the apostrophe without a trailing s.
   * *`<describe-poss what="$(this)">`* -- Describes $(this) in the possessive form, e.g. "his sword"
   * *`<describe-prop this="$(this)" what="property:name"/>`* -- Adds "a" "the" "an" to the text of the property appropriately.
   * *`<describe-view view="$(this)" cap/>` -- Will give the full description of the object, including any visible inventory. e.g. "A tall orc, weilding his sword."
   * *`<capitalize what="lower">`* -- will initial capitalize the text "lower" into "Lower"
   * *`<eaten what="$(this)"/>`* -- will output how much of the item has been consumed.
   
Most of these description Sam Tags have a variety of options, see SamSystem for details, but one of the most useful ones is the 'cap' option, which will capitalize the output. For instance:

   * *`<describe what="$(this)"/>`* -- This will output the brief of $(this) object, e.g. "A sword"


### Client Sam

The following SAM tags are useful for special features in the Alice and Zealous clients:

   * <atag tag="command"> text that you want themed with the color in the 'command' theme </atag>
   * <acmd tag="command" cmd="open west door">click here to open the west door.</acmd>
   

### Simple If Comparison

A simple if statement allows you to output one text if something is true, a different text if false.

```
   Look: [
      A statue of a man. The statue's face {? this.trait:expression |appears to be $(this.trait:expression) at you. |is expressionless. }
   ]
   Result(s):
      > look at statue
      A statue of a man. The statue's face is expressionless.
      > +setp statue "trait:expression similing
      Setting 'trait:expression' in <[Examples:complete:sam:statue]#4586> to "smiling".
      > look at statue
      A statue of a man. The statue's face appears to be smiling at you.
      >
```

In this example, if the property `trait:expression` doesn't exist (i.e. false), then "is expressionless" is output. If it does exist (true) then it is "appears to be smiling at you" will be output.


### Other Comparisons

There are quite a few inds of comparisons you can do in SAM:

BOOLEAN
```
    {? | $this.trait:variant |
        true text output |
        false text output }

    {? not | $this.trait:variant |
        true text output |
        false text output }
    {? equal | $this.trait:variant | ordinary |
        true text output |
        false text output }

    {? notequal | $this.trait:variant | ordinary |
        true text output |
        false text output }

    {? lessthen | $this.trait:variant | ordinary |
        true text output |
        false text output }

    {? lessthenorequal | $this.trait:variant | ordinary |
        true text output |
        false text output }

    {? greaterthen | $this.trait:variant | ordinary |
        true text output |
        false text output }

    {? greaterthenorequal | $this.trait:variant | ordinary |
        true text output |
        false text output }

    {? range | $this.trait:variant | lowervalue | uppervalue |
        true text output |
        false text output }
    
    Comment: range is exclusive, i.e. 
         {? range | 1 | 3 | 6 | true | false }"
            1   false
            2   false
            3   false
            3.1 true
            4   true
            5   true
            6   true
            6.1 false
            7   false
            8   false
            9   false


    {? when | $this.trait.variant |
        value1 | value1 equal $this.trait:variant |
        value2 | value2 equal $this.trait:variant |
        value3 | value2 equal $this.trait:variant |
        *      | any other value }
```


### Inline Merry

You can put Merry scripts inside of of SAM as follows:

```
   Look: [
      You are on a road. There are $[this.stars_in_sky + this.moons_in_sky] celestial bodies above you.
   ]
```

If the piece of Merry does not end in a semi-colon or a right-brace, it is assumed to be a 'statement' rather than an 'expression'. This means the code needs to explicitly return a value. For example,

```
   You are on a road. There are
     $[
         if (this.stars_in_sky > 5) {
             return "lots of";
         }
         if (this.stars_in_sky > 0) {
             return "a few";
         }
         return "no";
      ]
   stars in the sky.
```

### Inline Merry and Text

It can often be useful in SAM to manipulate text, so there are some very simple inline Merry functions that can perform this for you.

Text related:
   * *`$[capitalize("this is a test")]`* -- result: "This is a test"
   * *`$[proper("This is a test")]`* -- result: "This Is A Test"
   * *`$[decapitalize("This Is A Test")]`* -- result: "this Is A Test"
   * *`$[upper_case("This Is A Test")]`* -- result: "THIS IS A TEST"
   * *`$[lower_case("this Is A Test")]`* -- result: "this is a test"

Number related:
   * *`$[desc_cardinal(128)]`* -- result: "one hundred twenty-eight"
   * *`$[desc_ordinal(128)]`* -- result: "one hundred twenty-eighth"
   * *`$[comma(1024)]`* -- result: "1,024"


### Inline Merry and Comparisons

You can combine arbitrary Merry expressions for tests to use in comparisons, to make very sophisticated output. For example:

```
    {? | $[this.foo == "bar"] |
        true text output |
        false text output }

    {? | $[random(100)] |
        true one in 100 times. |
        false text output }

    {? | $[random(6)+random(6)+random(6)+3 == 18] |
        You rolled 3 sixes! |
        You didn't roll 3 sixes. }

    {? range | $[random(100)] | 25 | 75 |
        You rolled in the middle of the range. |
        You rolled outside the range. }
        
```

These can be quite sophisticated, for instance:

```
    {? | $[sizeof(Match($actor, "cigarette"))] |
    Actor has a cigarette. | Actor doesn't have cigarette. }
       Actor has a cigarette.
       Actor doesn't have a cigarette
       
```

### Common Inline Merry Uses

There are many things you can do in SAM by using inline Merry, but these are some of the most common:

   * *`$[Set($this, "my:property:name", 1);]`* -- Sets the property my:property:name in object $this to 1.
   * *`$[Set($this, "my:property:name", Get($this, "my:property:name) + 1); ]`* -- Adds 1 to property my:property:name on object $this



(not quite yet working examples)

```
You look at <describe what=$(this)> and your surroundings change!
   $[      EmitIn(Get($actor, "base:environment"), Describe($actor) + " looks at " + Describe($dbob) + " and disappears!\n", $actor);
         $actor."base:environment" = ${Examples:complete:desc:room-go-nowhere};
         $actor."base:proximity" = NewNRef(${Examples:complete:desc:room-go-nowhere}, "floor");
         $actor."base:stancestring" = "lying";
         $actor."base:prepositionstring" = "on";
         EmitIn(Get($actor, "base:environment"), Describe($actor) + " appears out of nowhere lying on the floor!\n", $actor);
   ]
```




