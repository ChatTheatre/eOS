SkotOS on the brink of the rewrite
==================================

(copied from http://www.alyx.com/zell/SkotOS.html dated Sun Aug 29 23:05:24 2004 )

A technical summary of the state of the SkotOS mudlib -- the beginning we've made on SkotOS 2.0, all the abilities that SkotOS 1.\* has, and perhaps ome discussion on how those abilities will survive the rewrite.

* * *

Components of SkotOS 2.0
------------------------

All versions of SkotOS run on DGD and rely on DGD's kernel library. SkotOS 1.\* requires the Skotos branch of DGD featuring network extensions, where SkotOS 2.0 will run on Dworkin's own distribution.

### The Base

All versions of SkotOS are modular. They all contain base code which implements ~System and basic toolkit libraries, and they all add modules onto the base to provide higher level functionality. Modules depend upon one another so that loading one may first trigger the automatic load of another, which in turn may trigger another, and so on. Before the release, some code may be moved out of ~System and into a new, unpriviliged user, e.g. ~SkotOS, but still considered part of the base.

Major services implemented in ~System:

*   The **program database** , a near-vital component of any persistent game running under DGD; records and remembers the inheritance dependencies of program issues upon one another. The primary purpose of this database is to allow the automatic recompilation of dependents when the code of a superclass needs to be recompiled.
*   A **clone database** whose task it is to enumerate all current clones of a given clonable. The current implementation switches automatically between an explicit and an implicit representation, depending on which it thinks is more efficient. The primary purpose of this database is for dataspace patching.
*   A **object name database** which implements a structured, filesystem-like name space for any object that which to take part. SkotOS virtual worlds are traditionally extremely data-driven, rather than class-driven; they tend to be configured clones rather than singleton classes. DGD-internal names such as e.g. '/world/obj/thing#23948' are useless to a developer, whereas 'World:Europe:Germany:Heidelberg:MarketSquareNW' makes a lot of sense, and allows e.g. the enumeration of all Heidelberg:\* objects.
*   A **patch database** in which we explicitly queue pending dataspace patches for any object in the game. Imagine that after three years of uptime we find we need to completely rewrite an abstract datatype upon which 60,000 objects rely. Program upgrading allows us to easily replace the implementation, but chances are the underlying data needs to be converted from the old format to the new. Unfortunately, swapping in 60,000 objects will freeze a game for hours and is an unrealistic proposition for a live game. Instead, we queue this patch for these 60,000 objects in the central patch database and use DGD's call\_touch() on each of them. Then, the patch occurs automatically and invisibly the next time each of these objects are naturally referenced. No maintenance downtime is required.
*   An **LPC value serializer** that converts any value to a string representation and back. LWO's that know how to export their state are automatically serialized as well, while persistent objects are referenced by their name. Recursive structures are reconstructed correctly. We use parse\_string() for speed.
*   A **port manager** that administers the ports opened by DGD. In addition to DGD's configuration file, we require an additional one which associates a label wich each port -- as in e.g. 'http' or 'admin logon'. External modules may then associate themselves with a given port label.
*   A **developer login** extension of the kernel library's, with an authentication database for developers entirely separate from whatever general user authentication scheme (often networked) a SkotOS instance uses for its general users. We also extend the wiztool with administrative commands for e.g. upgrading programs.
*   A **syslog daemon** that generalizes the reporting of runtime errors so that e.g. HTTP connections can compile a HTML view of the error. This daemon also uses a couple of tricks to pass compilation errors past the atomic barrier.
*   The idea of the **thread originator** , the value of this\_user() for a new thread, persisted through i.e. call\_out() and network callbacks. The originator is the user object that receives runtime error reports -- this allows, for example, HTTP requests that need network authentication to receive error traces.
*   Automatic **LWO stub creation** ; performing new\_object() on some program ./c/lib/foo automatically constructs and compiles a tiny program under ./c/lwo/foo and then performs the requested new\_object() on that program instead. This lets us avoid creating dozens and dozens of tiny concrete classes that do nothing but inherit pure programs. These stubs are similarly created under ./c/obj/foo when clone\_object() is used. The 'c' is for 'concrete', but still subject to change.

Apart from ~System, the base includes /lib which holds a number of pure toolkit libraries whose functions are all static and which have no internal state as well as a bunch of interfaces for abstract datatypes. Finally, under /c/lib, there reside a number of concrete implementations of those datatypes.

### The HTTP Module

This module answers HTTP requests on ports that can be claimed and configured by external entities, and converts them into LPC calls on those external port handlers. There's quite a bit to it, but in the interest of brevity suffice it to say that SkotOS games can be quite web-centric.

### The Merry Module

Merry, the in-game imperative scripting language, has emerged as possibly the single most important component in SkotOS. Not only individual games but also increasing amounts of the world simulation itself is written in Merry.

The language itself is merely a decorated and sandboxed form of LPC; the decorations are turned into 100% LPC, and the result is compiled. In other words, every Merry script becomes a program, with the obvious efficiency benefits.

### The SAM Module

Where Merry is imperative, SAM is functional. SAM -- essentially marked-up text -- always evaluates to a string, and should never have side-effects. SAM can also be seen as a dynamic template language like e.g. Java ServerPages and, indeed, dynamic HTML is as important an application of SAM as is e.g. room descriptions.

A lisp-like syntax controls the dynamic evaluation of SAM. The exact syntax is currently up in the air a bit (SAM is being completely reconsidered for SkotOS 2.0), but an (unimaginative) example room could look something like:

* * *

        You are in a pasture. You see \[size $cows\] cows here:
          \[foreach $cow in $cows do
            "A $(cow.colour) cow. "
          \]
        You also see an old abandoned farmhouse.
      

* * *

SAM is easily extended by in-game developers: commands such as 'foreach' and 'size' can be written for SAM in Merry. SAM can also inline Merry directly, though overuse of this feature is discouraged, as it tends to make the text rather cryptic.

Code from SkotOS 1.\*
---------------------

While the modules listed above form the solid foundation of all other Skotos code, they actually represent a fairly minor portion of the mudlib. Thousands upon thousands of lines of LPC will be first considered and then either dismissed or rewritten for inclusion in SkotOS 2.0. The rest of this document will be preoccupied with this older code.

### Verbs, Actions and Events

For text-based games, a good command parser is vital. While Skotos started out with a generally hard-coded grammar, at this point we've switched almost entirely to a very generic English syntax. Very briefly, we have:

        \[verb\] \[preposition 1\] \[noun phrase 1\] \[preposition 2\] \[noun phrase 2\] ...
        \[verb\] \[noun phrase 1\] \[preposition 2\] \[noun phrase 2\] ...
      

i.e.

        > wave at the sky with my sword
        > wave my sword at the sky
      

We also allow a single adverb before the verb, before any preposition, or after the last noun phrase, and the last portion of any command may be a free-form evocation, so e.g.

        > cheerfully wave my sword at the sky
        > cheerfully wave my sword at the sky, "We're victorious!!"
      

Over time, it became clear that this is a sufficiently powerful engine to implement just about any physical instruction a game could require. The benefits of having a single, centralized syntax for commands has been debated endlessly elsewhere and I will not go into it here; I take it for granted that the days of LPMud 'add\_action()' style parsing are long gone, and good riddance.

Architecturally, the parser belongs to the text interface module. The idea of the English language must not be central to a virtual world simulation. Thus we introduce the idea of an 'action'. For example, 'get' and 'take' are verbs that map to the same action. For simplicity, we call the action 'take' as well, but it is vital to remember that actions and verbs are entirely distinct concepts -- one is a medium-agnostic feature of the virtual world, the other is a syntactic construct. It sometimes helps to imagine a Quake engine interface, where e.g. double-clicking on a treasure chest might trigger a 'get' or 'open' action, without a verb ever being involved.

The idea of an event is fairly intuitive; something happens somewhere that is of interest to some objects. A developer who wants an object to react to some event will generally attach a Merry script to an aptly named property on an aptly chosen object. The details are too voluminous to include here.

Lots of things cause events to fire in SkotOS; the most important event source is probably the action. Whenever there is an in-world action, regardless of what triggered it -- a player verb, a NPC script, whatever -- an event is fired, or rather, four phases of the event:

*   First, there is a 'pre' phase event, which occurs after the game sanity-checks arguments, but before it does anything else. Interested parties may intercept this event, investigate the parameters, and potentially veto it -- if I attempt to wield a non-weapon, for example, the 'wield' action would helpfully fail and tell me why.
*   Second, there is a 'prime' phase event. Again, this event may be not only intercepted but also vetoed. This event is triggered when the system believes the action should proceed, so it's a last chance for scripts to veto -- for example, a cursed ring would veto a removal action.
*   Third, if the action is successful, there is the 'desc' phase event. The purpose of this phase is to describe the action. This, obviously, varies from game to game; a non-text-game has a very complicated describe phase. For a text game, the default action is for the System to compile a generic description of the event, i.e.
    
                    Zell cheerfully waves his sword to the sky, "We're victorious!!"'
                  
    
*   Finally, there is a fourth 'post' phase, which is triggered immediately after the fact of the event. This is the most common event to hook into to produce side-effects of actions that succeeded. If I drink a poisoned beverage, for example, the post-drink hook would start tormenting me with stomach cramps.

Describing the full event system in detail requires an entire document in itself. We shall have to stop with this simplistic summary -- except for one last observation --

The default output for the 'desc' action, described above, warrants further examination. Imagine that you are looking through the eyes of the sword -- or the sky -- you'd see something like,

        Zell cheerfully waves you at the sky, "We're victorious!!"
      

or

        Zell cheerfully waves a sword at you, "We're victorious!!"
      

The clever reader will have noted that even though I made a point earlier of separating the idea of verb and action, here we are with an action relying on verbs to construct default output. It is true that the dependency goes both ways and the relationship is complicated. As long as there is any text output in a game, even a Quake style game, something similar to verb-defining objects will be needed, even if the player never types a single line of text.

The engine that creates the correct output for each listener is incestously entwined with other components in SkotOS 1.\*, but will become a separate and fundamental part of SkotOS 2.0. It has quite a few interesting abilities, for which we do not have time here.

### Properties

Nearly all interrogation and manipulation of objects from within Merry and SAM is done through the **properties** exported by the object. Initially a simple mapping of string keys to mixed opaque values, properties have with time become complicated, dynamic beasts.

Each object still has only a single set of properties, and that set is still backed up by a single mapping that allows external objects to store and retrieve values. Overlaying this mapping, however, are **virtual properties** , the evaluation of which triggers underlying LPC. Quering for the 'base:environment' property of a physical object, for example, yields that object's current environment, even though the internal environment pointer is stored in a low-level LPC attribute.

Finally, setting a property on an object fires an event. This is the latest evolution of properties as the de-facto API of in-game objects, as it allows developers to hook complicated Merry scripts up to react to changes in properties. Thus setting the 'is-lit' property of a cigarette to true, for example, could kick-start the Merry script that lets it slowly burn down to the filter with atmospheric effects.

### Ur-parents

SkotOS objects tend to be luridly described and generally have a lot of space-consuming state associated with them. A well-crafted sword will have a details for the blade, the hilt, the edge, the pommel... each detail will have a brief, a long and an examine description. This adds up to a lot of text, and takes up a lot of space.

If there are six hundred players running around the world with identical swords, then, is it not foolish for each of their swords to contain exact redundant copies of the same descriptions?

Well, yes. Suicidal, in fact... and thus the need for "Ur-parents", the SkotOS approach to dataspace inheritance. LPC inheritance is not used in SkotOS to distinguish one world object from another -- all world objects are typically clones of exactly the same DGD object. The behaviour of an object is dictated by its state, not by its programmatic class. Ur objects allow clones to inherit each others state.

An object may have at most one parent, but a parent may have any number of children. Now if I clone a new, fresh, empty world object A and set its parent to S, the sword mentioned above, then **every query** for a non-transient attribute on A will in fact return the value held in S. In effect, I will be holding a duplicate of S, without pointlessly hogging memory for duplicated descriptions.

As A acquires state of its own, the values returned change. For some attributes, such as descriptions, A's state simply overrides that of S. For others, such as mass, multiplication occurs -- if A's local mass value is set to 1.1, the perceived mass of A is 110% of that of S.

In practice, a Skotos game consists of huge libraries of ur objects -- blueprints, so to speak -- that are never in actual use; never even acquire an environment. In the actual world, nearly every object -- player bodies included -- is a 'spawn'; a simple, near-empty child of some ur-parent.

### The Virtual World

SkotOS 1.\* had a fairly extensive virtual world simulation written in LPC. At this point, Merry and SAM are so powerful that for SkotOS 2.0, much of that simulation should be moved into the scripting layer. A partial feature list:

*   The basic **environment/inventory** idea: objects may be contained in an environment; an environment can be queried for a flat or a recursive content list.
*   **Details** establishes the idea of a physical object as distinct from that of a stand-alone DGD object. A fireplace in a room, for example, is not represented as its own object, but the parser and description engines see little distinction between it and e.g. a sword in your hand.
*   **Proximity** is an extension of environment; objects are not only contained in an environment, but may also be near something else in that environment -- a detail or another contained object. While quite effective in practice, proximity has always been a troublesome component of our code. If both A and B are near C, how near are A and B to one another? If A is near B and B is near C, is A near to C? I doubt we'll drop proximity from SkotOS 2.0, but we may wish to redesign it.
*   The **stance/preposition** of an object is as basic a component of its state as environment and proximity: the sword is lying on the table; the chair is sitting on the floor; four balloons float near the ceiling. When an object moves in SkotOS, it does so not to a new environment but to a new environment/prox/stance/preposition.
*   We have a rather nice **light** system; physical objects have a luminosity setting that determines how much light they emit. The luminosity of a container is the linear sum of the luminosity of its contained objects, provided it's either translucent or open. The light level of e.g. a room, finally, is a function of the total luminosity in it and the room's size (see bulk below). Events are triggered, obviously, when light levels change.
*   Objects have **bulk** too; volume, density and mass (one of which is computed from the other two). The mass of a container, obviously, is the linear sum of the masses of its contents, whereas its volume may stay static if the container is rigid -- e.g. a crate.

In addition, there's a rather involved **clothing/weapon** system where articles of clothing cover subsets of body parts and obscure one another; there's low-level support for safe **trading** -- is object A being offered by object B to object C? There's generic **skill** support, the **sex** of an object, and a few other attributes that no longer belong -- if they ever did -- in the same abstraction layer as the physical simulation.

* * *