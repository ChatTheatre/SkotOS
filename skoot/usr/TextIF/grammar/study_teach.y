<COMMAND verb="study" keywords="with">
    <FORMAT value="%s"
            function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }) })"/>
    <FORMAT value="%s Words 'with' Ob"
            function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }), tree[1], tree[3] });"/>
    <FORMAT value="%s 'with' Ob"
            function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }), nil, tree[2] })"/>
    <FORMAT value="%s Words"
            function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }), tree[1] })"/>
    <FORMAT value="%s Words Ob"
            function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }), tree[1], tree[2] })"/>
</COMMAND>
<COMMAND verb="teach">
    <FORMAT value="%s"
            function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }) })"/>
    <FORMAT value="%s Words 'to' Ob"
            function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }), tree[3], tree[1] });"/>
    <FORMAT value="%s Ob Words"
            function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }), tree[1], tree[2] })"/>
    <FORMAT value="%s Ob"
            function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }), tree[1] })"/>
</COMMAND>
<COMMAND verb="skills">
    <FORMAT value="%s"      function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }) });"/>
    <FORMAT value="%s Word" function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }), tree[1] });"/>
</COMMAND>
<COMMAND verb="assess" keywords="for">
    <FORMAT value="%s"                function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }) });"/>
    <FORMAT value="%s Ob 'for' Words" function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }), tree[1], tree[3] });"/>
    <FORMAT value="%s Ob"             function="return ({ ({ \"/usr/TextIF/sys/cmds/study_teach\", tree[0] }), tree[1] });"/>
</COMMAND>
