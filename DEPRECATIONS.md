# To Deprecate

This is code or WOE trees that should be deleted from Gables

## StoryNuggets

None of the other games uses the old `Theatre:Theatres:Gables` `StartStory:Nuggets` method. Instead, we'll be using @saraht45's MVP start story.
  * [ ] Use nuggets for inspiration for MVP startstory
  * [ ] Delete unused Theatre:Nuggets
  * [ ] Ask @saraht45's advice if there is ANY value to them
  * [ ] If no value, remove them from SID and eventually from the code for SkotOS lib.

## Bilbo & Generic

Bilbo is the original scripting language for SkotOS, and should be deprecated in favor of Merry. However, there are old objects in the game like Generic:* that still have these scripts in them. Any in Generic that are not already converted to Shared should be converted and deleted.

At some point after we can cold boot, we need to search and delete all bilbo-related code in SkotOS.

