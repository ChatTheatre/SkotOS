<!-- Commands related to the CSD module. -->

<!-- FIGHT -->
<COMMAND verb="fight">
  <FORMAT value="%s"
          function="return ({ ({ \"/usr/CSD/sys/cmds/csd\", \"fight\" }) })"/>
</COMMAND>

<COMMAND verb="fight">
  <FORMAT value="%s Obs"
          function="return ({ ({ \"/usr/CSD/sys/cmds/csd\", \"fight\" }), tree[1] })"/>
</COMMAND>

<!-- SURRENDER -->
<COMMAND verb="surrender">
  <FORMAT value="%s"
          function="return ({ ({ \"/usr/CSD/sys/cmds/csd\", \"surrender\" }) })"/>
</COMMAND>

<COMMAND verb="surrender">
  <FORMAT value="%s Obs"
          function="return ({ ({ \"/usr/CSD/sys/cmds/csd\", \"surrender\" }), tree[1] })"/>
</COMMAND>

<!-- SKILLS -->   
<COMMAND verb="skills">
  <FORMAT value="%s"
          function="return ({ ({ \"/usr/CSD/sys/cmds/csd\", \"skills\" }) })"/>
</COMMAND>

<!-- STATS -->
<COMMAND verb="stats">
  <FORMAT value="%s"
          function="return ({ ({ \"/usr/CSD/sys/cmds/csd\", \"stats\" }) })"/>
</COMMAND>

<!-- AUTOCOMBAT -->
<COMMAND verb="autocombat">
  <FORMAT value="%s"
          function="return ({ ({ \"/usr/CSD/sys/cmds/csd\", \"autocombat\" }) })"/>
</COMMAND>
