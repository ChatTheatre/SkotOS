# Merry Tips

## What Arguments?

Quite often you'll run into a situation where you want to understand what arguments are being sent to the signal that your Merry code is working under.

The quickest way to do this is to put this short code into the signal, for example:

```
    > +tool merry edit %Examples:workinprogress:sam:sam-toy react:probe-dob
    EmitTo( $actor, dump_value( args ) ) ;

    > probe toy

    You approach a toy.
    ([ "actor":<Chatters:LC:ch:allen-christopher>,
       "dob":({ (43)O(/base/data/nref#-1, <[Examples:workinprogress:sam:sam-toy]#11527>, "default") }),
       "dob:description":"probe",
       "dob:distance":3,
       "imperative":"probe",
       "obscurity":0,
       "origtarget":"none",
       "parts":({ ({ "V" }), ({ "O", ({ (43)O(/base/data/nref#-1, <[Examples:workinprogress:sam:sam-toy]#11527>, "default") }) }) }),
       "preps":([ ]),
       "privacy":0,
       "rob":</base/data/result#-1>,
       "roles":({ "dob" }),
       "target":(43)O(/base/data/nref#-1, <[Examples:workinprogress:sam:sam-toy]#11527>, "default"),
       "this":<[Examples:workinprogress:sam:sam-toy]#11527>,
       "verb":"probe",
       "vob":<Socials:Verbs:probe>,
       "witnesses":({ <Examples:complete:desc:room-go-nowhere>, ({ }) }) ])
    >
```

-- [Christopher Allen]() - 01 Mar 2003

You can also put inline Merry in descriptions to see what arguments are available in SAM:

```
   Look: [
      $[dump_value( args )]
   ]
   React-post:push-dob: [
      $[dump_value( args )]
   ]

   > look at toy's button
   ([ "actor":<Chatters:LC:ch:allen-christopher>,
      "looker":<Chatters:LC:ch:allen-christopher>,
      "node":</usr/SkotOS/data/merry#-1>,
      "this":<[Examples:complete:sam:sam-toy]#11527> ])
   > push toy's button
   You push a button.
   ([ "actor":<Chatters:LC:ch:allen-christopher>,
      "dob":({ (43)O(/base/data/nref#-1, <[Examples:complete:sam:sam-toy #11527>, "button") }),
      "dob:description":"push",
      "dob:distance":3,
      "imperative":"push",
      "ldef":X[S] $(you) $(verb) $(dobname).,
      "node":</usr/SkotOS/data/merry#-1>,
      "obscurity":0,
      "origtarget":"none",
      "parts":({ ({ "V" }), ({ "O", ({ (43)O(/base/data/nref#-1, <[Examples:complete:sam:sam-toy]#11527>, "button") }) }) }),
      "preps":([ ]), "privacy":0, "rob":</base/data/result#-1>,
      "roles":({ "dob" }),
      "target":(43)O(/base/data/nref#-1, <[Examples:complete:sam:sam-toy]#11527>, "button"),
      "this":<[Examples:complete:sam:sam-toy]#11527>,
      "verb":"push",
      "vob":<Socials:Verbs:push>,
      "witnesses":({ <Examples:complete:desc:room-go-nowhere>, ({ }) }) ])
      >
```

-- [Christopher Allen]() - 13 Mar 2003

You can even use this dump with +tool to test things out:

```
   > +tool sam exec $[dump_value( args )]

   Original input:
           $[dump_value( args )]

   Output:
           "([ \"actor\":<Chatters:LC:ch:allen-christopher>, \
                "body\":<Chatters:LC:ch:allen-christopher>, \
                "here\":<Examples:complete:desc:room-go-nowhere>, \
                "looker\":<Chatters:LC:ch:allen-christopher>, \
                "node\":</usr/SkotOS/data/merry#-1> ])"

   > +tool merry exec EmitTo( $actor, dump_value( args ) ) ;
   ([ "actor":<Chatters:LC:ch:allen-christopher>,
      "body":<Chatters:LC:ch:allen-christopher>,
      "here":<Examples:complete:desc:room-go-nowhere>,
      "looker":<Chatters:LC:ch:allen-christopher> ])

   Merry code:   EmitTo( $actor, dump_value( args ) ) ;
   Merry result: nil
```

-- [Christopher Allen]() - 16 Mar 2003

## Debugging Signals, setprop-post

It can be very useful when debugging an object with many signals, in particular setprop-post signals, to use the following debug code:

```
if ( Get(this,"debug") ){
   EmitTo(${Chatters:LC:ch:allen-christopher},"SIGNAL: react-post:drop-what\n" + dump_value( args ) + "\n");
}
```

Just `+setp object "debug true"` on the object while you are debugging it, and you'll see all the signals.

```
   > drop torch
   You drop a flaming torch.
   SIGNAL: react-post:drop-what
   ([ "action":"command",
   "actor":<Chatters:LC:ch:allen-christopher>,
   "imp":"drop",
   "imperative":"drop",
   "ldef":X[S] $(you) $(verb) $(whatname).,
   "origtarget":"none",
   "phase":"post",
   "rob":</base/data/result#-1>,
   "roles":({ "what" }),
   "target":(43)O(/base/data/nref#-1, <[Examples:incomplete:light:torch]#9591>, "default"),
   "this":<[Examples:incomplete:light:torch]#9591>,
   "verb":"drop",
   "vob":<Neoct:Base:Verbs:drop>,
   "what":({ (43)O(/base/data/nref#-1, <[Examples:incomplete:light:torch]#9591>, "default") }),
   "what:description":"drop",
   "witnesses":({ <Examples:complete:desc:room-go-nowhere>, ({ }) }) ])
```

-- [Christopher Allen]() - 06 Apr 2003

\> How I send theme tags in emits? I want to make this more readable.

Try something like:

```
   EmitTo(${Chatters:LC:ch:allen-christopher},
          TAG("SIGNAL: act:start\n" + dump_value(args),
              "debug"));
```

and then go associate a colour with the 'debug' tag in your favourite theme. In other words, instead of str, do TAG(str, tag).

-- [ParWinzell]() -- 07 April 2003

## Setting/Deleting Properties

00:20 [Marrach:CC|Kylass] "What's the command if I want to delete a property? Is this correct: Set($actor."death:timestamp", = nil);"

00:20 [Marrach:CC|Stickyjellypoo] "Set($actor,"death:timestamp", nil); I do believe"

00:20 [Marrach:CC|Xanom] "No. Set ($actor, "death:timestamp", nil); abbreviated to $actor."death:timestamp" = nil; if you prefer."

00:22 [Marrach:CC|Xanom] "You only need to use the Set() form if you want to construct the property string dynamically."

-- [Christopher Allen]() - 01 Mar 2003

## Woe Names in Merry

00:29 [CC|Umbric] "Okay, for getting a specific objects from WOE, you can use $(This:Is:a-woe-object), and {This:Is:a-woe-object}. What's the difference?"

21:22 [CC|Christophera] "The $(This:Is:a-woe-object) form is a fixed reference, it will not change of the object it targets changes its name. {This:Is:a-woe-object} the value will change of the object changes."

21:21 [CC|Umbric] "Actually, apparentl ${ } works too."

21:23 [CC|Umbric] "The problem cropped up in Spawn."

21:23 [CC|Umbric] "Spawn({woename}) didn't work...Spawn($(woename)) didn't work....Spawn(${woename}) worked."

## When to use False in Delays

00:32 [Marrach:CC|Kylass] "If I'm using an if or else, where do I put the return False; (or is it TRUE?) so that the action will not be seen for the 'if'?"

00:32 [Marrach:CC|Stickyjellypoo] "I usually tack on at the beginning a $delay(.5,FALSE);"

00:33 [Marrach:CC|Xanom] "In principle a bad idea."

00:33 [Marrach:CC|Stickyjellypoo] "What's a better one?"

00:34 [Marrach:CC|Xanom] "A $delay(x, FALSE) in the react:??? phase probably won't do what you expect."

00:38 [Marrach:CC|Xanom] "Very quick summary: pre (react-pre:...) is 'check this before starting the action' (false aborts action), std (react:...) is do this as the action is about to start (false stops further processing), -desc (react-desc:...) is before action is described (false stops further descriptions but action continues), -post (react-post:...) is after the action (return value is ignored)."

00:40 [Marrach:CC|Xanom] "Most of the time, you should use -post or -desc, unless you actually are modifying the action itself. Certainly, any 'you do this, and then X happens' is -post."

-- [Christopher Allen]() - 01 Mar 2003

More info in [PhasesActionsTutorial]() -- [SpZiph]() - 20 May 2003

## Forcing an Movement

08:05 [CC] "Steeev waves. "Good morning! Anybody know an Act(), Social(), or other command that I can use in Merry to get $actor to go through an exit?"

13:19 [CC|Zell] "Steeev: Act($actor, "enter", $what: exitnref)"

-- [Christopher Allen]() - 05 Mar 2003

## Presence of an Item

\> I want to make it so that unless you have a lighter in your inventory,
\> you can't light anything. This would also fall into the idea of 'no
\> belt...no wearing of items needing a belt'. Not sure how to approach
\> this one either.

In Merry, e.g.

```
   Match($actor, "lighter")

will return an array of everything you are carrying that matches the
noun "lighter". Thus a check like

   if (sizeof(Match($actor, "lighter")) > 0) {
     /* the actor has a lighter */
   } else {
     /* the actor doesn't */
   }
```

should do the trick...

-- [ParWinzell]() - 12 Feb 2003

## +Tool Eval & Exec

It can be useful to do test various short Merry scripts using the +tool command. The commands are:

**+tool merry exec:** Executes a piece of Merry code. Any value returned will be displayed to the user. Example:

```
         +tool merry exec int f,i; f=1; for(i=2;i<14;i++) f*=i; return f;
```

**+tool merry eval:** Evaluate a Merry expression, which is basically a shortcut for doing this:

```
         +tool merry exec return <your expression here>;
```

Examples:

```
         +tool merry eval (104 - 32) * 5 / 9
         +tool merry eval 100.0 * 1.04 * 1.04 * 1.04
```

A number of arguments are available to "+tool merry eval". You can see them with the following:

```
         >+tool merry eval dump_value( args )
         Merry code:   dump_value( args )
         Merry result: "([ \"actor\":<Chatters:LC:ch:allen-christopher>,
                           \"body\":<Chatters:LC:ch:allen-christopher>,
                           \"here\":<Examples:complete:desc:room-go-nowhere>,
                           \"looker\":<Chatters:LC:ch:allen-christopher> ])"
```

As you can see, $this, $body, $actor and $looker are set to your own body, and $here refers to the environment you are in.

Please post your favor +tool exec and eval tips here!

-- [Christopher Allen]() - 02 Mar 2003

## Tokenising Strings

explode(" a b c", " ") -> ({ "", "a", "b", "", "c" })

Explode basically blows a string up by a given delimiter, yielding empty string when there are multiple delimiters in a row. To skip multiples, just subtract ({ "" }) from the result. Explode's counterpart is

implode(({ "foo", "bar" }), ", ") -> "foo, bar"

Merry also has access to the extremely powerful parse_string() function in DGD. Beware: working with parse_string() is like writing a compiler in lex/yacc format, and then double the complexity. It does, however, do any kind of parsing you could reasonably ask for. There is a slightly user-friendly tutorial here:

<https://web.archive.org/web/20050209083034/http://www.mindspring.com/~zeppo1/parse_string.html>

-- [ParWinzell]() - 14 Oct 2002

## Limiting by Count

14:21 [Marrach:CC|Stickyjellypoo] "Anyone know of any items in woe that have a counter set into them? For example, a normal drink holds four drinks in it. Are there any other container type items with a counter script in them that I could take a peek at?"

14:22 [Marrach:CC|Zwoc] "Well, what kind of counter are you in need of?"

14:22 [Marrach:CC|Stickyjellypoo] "I'm trying to limit the amount of pinches inside of my ithcing powder sack/"

14:23 [Marrach:CC|Zwoc] "In the react script. this.counter ++; if( this.counter > 10 ){ EmitTo( $actor, "The sack is empty." ); return FALSE; }"

14:23 [Marrach:CC|Zwoc] "Where 10 is the number of pinches you want."

14:25 [Marrach:CC|Stickyjellypoo] "That's it? :)"

14:25 [Marrach:CC] "Zwoc nods."

14:25 [Marrach:CC|Zwoc] "And after that little snippet, the rest of the script goes."

-- [Christopher Allen]() - 29 Mar 2003

## OneOf in Merry

Try

```
     processed_string = UnSAM(ParseXML("You are {happy|sad}."));
```

This is equivalent to $sam(You are {happy|sad}.) in Bilbo. The ParseXML call translates a string into the binary format (which is what is stored in e.g. object descriptions) and UnSAM is the call that renders a string from it.

-- [ParWinzell]() - 22 Jun 2002

You can also:

```
   string * lightoptions;
   lightoptions = ({ "a", "b", "c" });
   EmitTo ($actor, "It is very " + lightoptions[ random (sizeof (lightoptions)) ] + " in here.");
```

One word of warning. Don't put a $delay between the lightoptions = ({ ... }); and where it is used (in the emit). Alternatively, call it $lightoptions. I won't go into the tradeoffs between them at this stage.

-- [SpZiph]() - 16 March 2003

These days, the above can be written as such:

```
   EmitTo( $actor, "It is very " + oneof( "a", "b", "c" ) + " in here." );
```

Alternatively,

```
  string lightoptions;
  lightoptions = oneof( "a", "b", "c" );
  EmitTo( $actor, "It is very " + lightoptions + " in here." );
```

-- [KalleAlm]() - 05 Mar 2004

## Does Woe:Name Exist?

05:05 [CC|Xanom] "Anyone know how to test 'if WOE:NAME exists?"

05:10 [CC|Kargh] "I would think that if(woe:name) { should work. But I never tested that."

05:13 [CC|Xanom] "No. The form ${woe:name} and OBJ("woe:name") both fail. And "woe:name" will trivially pass, since it's a non-empty string."

10:56 [S7:CC|Zell] "Nom -- it's find_object("woe:name"). I should probably add second boolean arguments to all the casting functions Obj(), Int(), etc, that instruct them to return e.g. nil rather than throw an error."

-- [Christopher Allen]() - 30 Apr 2003

## First Character in a String?

13:38 [CC|Xanom] "How do I take the first char of a string in merry?"

13:39 [CC|Caltosdarr] "string[0 .. 0]"

13:39 [CC|Xanom] "Thanks"

13:41 [S7:CC|Zell] "Well, first char is str[0] (an integer); str[0 .. 0] is just a subset (a string). "foo"[0] `= 'f' =` 102 while "foo"[0 .. 0] == "f"

-- [Christopher Allen]() - 30 Apr 2003

## Last Word in a String?

14:10 [CC|Xanom] "I know I've asked this before, but if I have a string, what's the easiest way to get the last 'word' in the string."

14:11 [S7:CC|Zell] "arr = explode(str, " "); return arr[sizeof(arr)-1]"

14:11 [S7:CC|Zell] "although that doesn't do any punctuation stripping or anything"

-- [Christopher Allen]() - 30 Apr 2003

## Catch

```
----- Original Message -----
From: "Par Winzell" <zell@skotos.net>
To: <skotos-seven@skotos.net>
Sent: Sunday, September 01, 2002 4:58 PM
Subject: Re: [skotos-seven] Merry scripts running time?


Kalle,

> The thought struck me -- for how long does merry scripts run, anyway?

They run forever, unless they either A) do a 'return' rather than a
$delay, or B) cause a run-time error, which aborts the execution.

> Some tests prove that scripts do not run forever. Especially in the
> case of a script ran in a character body.
>
> I had a script that occasionally made silly evokes in my environment, in
> a never-ending loop (with about 15 mins apart each evoke).
>
> I found the script was no longer running when I'd been logged out for
> some time.

This is a good case of B) -- when you log out, your environment becomes
'nil'. I bet your code sent the $actor's base:environment to a function
like perhaps EmitIn, which freaked out and triggered an error, aborting
the execution of the script.

There are various ways to make these things more sturdy. One is to catch
errors e.g. like this --

   while (TRUE) {
     catch {
       EmitIn($actor."base:environment", "Time passes...\n");
     }

     $delay(10);
   }

All code inside the catch {} statement will run as usual unless there is
a runtime error, in which case execution does not abort entirely -- but
rather jumps to the end of the catch block. In other words, the EmitIn
is safely encapsulated, so even if it breaks, the loop is guaranteed to
continue running.

Note that in this particular case, that would be an IDIOTIC solution to
the problem -- the correct thing to do is to check if base:environment
is nil before emitting in it! Otherwise the error logs will fill up with
un-ending streams of caught errors...


Another way to solve this problem is by using some Merry functions that
I wrote before I figured out how to do $delay, but which I haven't been
pushing on you guys since. There are reasons to use them occasionally,
though, because they provide a more robust way to do timing-sensitive
stuff like this. I'll try to remember to document them in the next few
days. Meanwhile, A) do checks on everything you think might break -- in
most cases, sending nil's to functions that don't know what to do with
them -- and B) sparing use of catch() statements around critical parts
of infinite loops, like the above.

Zell
```

-- [ParWinzell]() - 01 September 2002

18:48 [CC|Kargh] "Here is a better question, does the Spawn() actually happen (when used with catch())? If so, is there a way to do a "mock" spawn to check if its a valid spawnable object first?"

19:03 [S7:CC|Zell] "No, when you catch it, the spawn fails; the error occurs, but you regain control just after the catch."

19:04 [CC|Seidl] "? if (0 != catch($newobj = Spawn(Obj(woename)))) { the spawn failed } the spawn worked."

19:04 [S7:CC|Zell] "Almost, nil instead of 0 :)"

19:05 [CC|Seidl] "The docs specificly say it returns 0 or a string, but o.k."

19:05 [S7:CC|Zell] "What docs?"

19:05 [CC|Seidl] "MerryFuns#catch"

19:06 [CC|Seidl] "Says it returns the error code or 0. But at least I was close."

19:07 [S7:CC|Zell] "It's incorrect. Dunno who added that. Looks like an obsolete kfun document."

19:07 [S7:CC|Zell] "Yep, you were essentially right :)"

19:08 [CC|Seidl] "Its a little evil that catch uses ()'s instead of {}'s, but I expect thats a parser limitation."

19:11 [S7:CC|Zell] "there's two versions; err = catch(blah blah), and catch { blah blah } : { do this if there is an error; }"

19:12 [CC|Seidl] "Interesting. We really could use docs on that kind of thing."

19:12 [S7:CC|Zell] "Yep."

-- [Christopher Allen]() - 30 Apr 2003

## Performance

```
----- Original Message -----
From: "Par Winzell" <zell@skotos.net>
To: <skotos-seven@skotos.net>
Sent: Monday, November 04, 2002 4:41 PM
Subject: Re: [skotos-seven] SAM descriptions (again,sorry)


Carl,

>> In reality this would get pretty sluggish; we can talk about how to
>> optimize it in a follow-up mail if you decide to use these hooks.
>>
>> And I do suggest you use them.
>
> ok then, lets talk optimization :)
> I'm going for an approach that will create something along the lines of
> trait:color:description for each trait and these properties will be used
> in simple SAM for the brief, look and examine.

Let's look at two extreme scenarios.

  A) For each trait:blah, you want to create trait:blah:description. In
this case you have nothing to worry about. Each trait:blah will have a
script associated with it that does just what's required.

  B) For each trait:blah, you call i.e. descsub. This is bad because in
many cases, 20 traits will be executed in a single thread (at once) and
each one would trigger a new call to descsub (which is a fairly complex
routine, as I recall). Only one call is required. In this case, there is
need to do something like,

merry:lib:maybe-descsub =
X[M]
   if (this."descsub-already-scheduled") {
     return;
   }
   this."descsub-already-scheduled" = TRUE;
   $delay(0);
   Call(this, "do-descsub");

merry:lib:do-descsub =
X[M]
   Call(this, "descsub");
   this."descsub-already-scheduled" = FALSE;


If you start off doing A) but slowly add components from B), you need to
keep this in mind so that expensive algorithms aren't needlessly run
more than once.

Zell
```

## Alternative to $delay: every() in()

------

Original Message ----- From: "Par Winzell"
