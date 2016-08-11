/*======================================================================
   Project:  The Cup-O MUD Client
   File:     CupOmud.java
   Language: Java 1.0
   Author:   Alex Stewart
   Date:     Jun 8, 1997
   Version:  1.3
   Copyright 1996,1997 Alex Stewart, all rights reserved
   This file is licensed under the terms of the GNU Public License.
   See the accompanying COPYING file for details.
 =======================================================================
 This is the main class composing the Cup-O MUD client applet.
 When started this applet attempts to load a locally-installed java MUD
 client class (Cup-O MUD or otherwise) as mudclient.Applet.  If it
 doesn't find one in the local CLASSPATH, the browser will fetch
 mudclient/Applet.class from the remote site, which will simply throw an
 exception to tell this class to use its default routines instead.  It
 sounds a bit complicated, but it's really the only way there is to
 attempt to run a local version if there is one installed without
 crashing horribly if there isn't one..
 
 It should be noted here that this class is NOT a good example of Object
 Oriented Programming.  It seems a shame to take a nice OO language like
 Java and completely abandon nearly all OO aspects of it, but this was
 nonetheless done deliberately for reasons of efficiency and code size.
 Due to the (stupid) way Java was designed, each object class is its own
 file, which must be downloaded individually by the browser at load
 time.  As this requires establishing a separate net connection for each
 class of a Java applet, and establishing a connection is often the most
 time-consuming portion of network communications, creating an applet of
 this size in a properly object oriented fashion (which would involve
 quite a few class files) would cause it to download a _lot_ slower.  As
 part of the goal of this project was to produce a quick-loading applet,
 it has therefore been kept to as few classes (and thus as un-OO-like)
 as possible.
 
 Some other aspects of this class may also be a little bit less than
 verbose or nicely structured, too, as I did do some scrunching in order
 to optimize size of the applet in favor of nicely structured
 programming.  Unfortunately, that's just the way things are sometimes,
 and you've gotta go with the priorities.

 Revision History:
   1.0 - Initial Release
   1.1 - Significantly redesigned the applet's structure to allow it to
         work properly with Netscape 3.0's new (less flexible) Java
         engine.
   1.2 - Bug fix for problem with StringIndexOutOfBoundsExceptions
         when parsing quoted strings in MCP requests.
   1.3 - Bug fix for incorrectly using "mcp" parameter instead of "telecho"
         parameter to set paramTelEcho.
         Added conditional-compile debugging statements to source.
*/

//*#debugall: +debugstart +debugflow +debugt2 +debugevent +debugio */
//*#debugall: +debugtel +debugmcp +debugout +debugwrap +debugansi */

//*#debugstart || debugflow || debugt2 || debugevent || debugio: +debug */
//*#debugtel || debugmcp || debugout || debugwrap || debugansi:  +debug */

import netscape.javascript.*;
import java.awt.*;
import java.applet.Applet;
import java.io.*;
import java.net.*;
import java.util.*;

public class CupOmud extends Applet implements Runnable {

   public static final String interfaceType = "Cup-O MUD";
   public static final int    interfaceRevision = 1;

// ================================ Constants ================================

   private static final int WRAP_MARGIN          = 20;   // Max word size to attempt to wrap
   private static final int NET_BUFFER_SIZE      = 8192; // 8K network buffer
   private static final int BACKSCROLL_OVERDRAFT = 2048; // 2K overdraft
   private static final int DEFAULT_BACKSCROLL   = 12288;// 12K
   private static final int DEFAULT_RECALL       = 20;   // 20 commands
   private static final String DEFAULT_FONTNAME  = "Courier";
   private static final int    DEFAULT_FONTSTYLE = Font.PLAIN;
   private static final int    DEFAULT_FONTSIZE  = 10;
   
// ============================= Global Variables ============================

   private String  paramHost, paramName, paramEcho, paramCommand,
                   paramURLtarget, paramWaitfor;
    private String killBuffer;
   private int     paramPort, paramRows, paramCols, paramBackscroll,
                   paramRecall;
   private boolean paramMCP, paramMCPauth, paramTelEcho;
   private Color   paramBGcolor, paramOBGcolor, paramOFGcolor,
                   paramIBGcolor, paramIFGcolor;
   private Font    paramFont;
   private int[]   paramHcorrect = {100, 0};
   private int[]   paramVcorrect = {100, 0};

   /* SKOTOS */
   JSObject        browserWindow;
   JSObject        browserDocument;
   JSObject	   mapImage;

   private String  userName, passWord, charName;
   /* END SKOTOS */

   private static final Applet remote = null;
   private int     charHeight, charWidth;
   private int     outWinBHeight, outWinBWidth;
   private int     screenRows, screenCols;
   private Thread  mainThread;
   private TextArea  outputWindow;
   private TextField inputWindow;
   private String  history[];
   private int     historyPos;
   private Socket  netConnection = null;
   private boolean mcp_enabled   = false;

// =============================== Constructor ===============================

   public CupOmud() {
      /* Try to load the local version of a mudclient.  If it succeeds,
       * 'remote' will be used for all applet functions.  If it doesn't
       * succeed, an exception will be thrown and we'll continue on as
       * normal. */
   }

// ========================= Public Interface Methods ========================
   public String getAppletInfo() {
//*#debug*/ if (remote != null) return "Cup-O MUD v1.3 loader (debug version): " + remote.getAppletInfo();
//*#debug*/ return "Cup-O MUD v1.3, Copyright 1997 Alex Stewart (debug version)";
/*#!debug*/ if (remote != null) return "Cup-O MUD v1.3 loader: " + remote.getAppletInfo();
/*#!debug*/ return "Cup-O MUD v1.3, Copyright 1997 Alex Stewart";
   }


   public void init() {
      System.err.println("init() called");

//*#debugstart || debugflow*/ debug("init() called.");
      /* Are we being called as a preload? */
      if (parseBool(getParameter("preload"), false)) {
//*#debugstart*/ debug("init: applet preload.  done.");
         return;
      }

      if (remote != null) { remote.init(); return; }

      System.out.println(getAppletInfo());

      /* Get the applet parameters and sort them out... */
      paramHost = getParameter("host");
      if (paramHost == null) paramHost = getCodeBase().getHost();
      paramPort = parseInt(getParameter("port"), 0);
      paramName = getParameter("sysname");
//*#debugparam*/ debug("init: sysname="+paramName);
      if (paramName == null) paramName = paramHost+":"+paramPort;
//*#debugparam*/ debug("init: host="+paramHost);
//*#debugparam*/ debug("init: port="+paramPort);
      paramRows = parseInt(getParameter("rows"), 0);
//*#debugparam*/ debug("init: rows="+Integer.toString(paramRows));
      paramCols = parseInt(getParameter("cols"), 0);
//*#debugparam*/ debug("init: cols="+Integer.toString(paramCols));
      paramEcho = getParameter("echo");
//*#debugparam*/ debug("init: echo="+paramEcho);
      paramCommand = getParameter("command");
//*#debugparam*/ debug("init: command="+paramCommand);
      paramMCP = parseBool(getParameter("MCP"), false);
//*#debugparam*/ debug("init: MCP="+(paramMCP?"true":"false"));
      paramMCPauth = parseBool(getParameter("MCPauth"), false);
//*#debugparam*/ debug("init: MCPauth="+(paramMCPauth?"true":"false"));
      paramURLtarget = getParameter("URLtarget");
//*#debugparam*/ debug("init: URLtarget="+paramURLtarget);
      paramTelEcho = parseBool(getParameter("telecho"), true);
//*#debugparam*/ debug("init: telecho="+(paramTelEcho?"true":"false"));
      paramFont = parseFont(getParameter("font"), new Font(DEFAULT_FONTNAME, DEFAULT_FONTSTYLE, DEFAULT_FONTSIZE));
//*#debugparam*/ debug("init: font="+paramFont);
      paramWaitfor = getParameter("waitfor");
//*#debugparam*/ debug("init: waitfor="+paramWaitfor);
      paramOBGcolor = parseColor(getParameter("output_bgcolor"), null);
//*#debugparam*/ debug("init: output_bgcolor="+paramOBGcolor);
      paramOFGcolor = parseColor(getParameter("output_fgcolor"), null);
//*#debugparam*/ debug("init: output_fgcolor="+paramOFGcolor);
      paramIBGcolor = parseColor(getParameter("input_bgcolor"), null);
//*#debugparam*/ debug("init: input_bgcolor="+paramIBGcolor);
      paramIFGcolor = parseColor(getParameter("input_fgcolor"), null);
//*#debugparam*/ debug("init: input_fgcolor="+paramIFGcolor);
      paramBGcolor = parseColor(getParameter("bgcolor"), null);
//*#debugparam*/ debug("init: bgcolor="+paramBGcolor);
      paramHcorrect = parseCorrect(getParameter("hcorrect"), paramHcorrect);
//*#debugparam*/ debug("init: hcorrect="+paramHcorrect);
      paramVcorrect = parseCorrect(getParameter("vcorrect"), paramVcorrect);
//*#debugparam*/ debug("init: vcorrect="+paramVcorrect);
      paramBackscroll = parseInt(getParameter("backscroll"), DEFAULT_BACKSCROLL);
//*#debugparam*/ debug("init: backscroll="+Integer.toString(paramBackscroll));
      paramRecall = parseInt(getParameter("recall"), DEFAULT_RECALL);
//*#debugparam*/ debug("init: recall="+Integer.toString(paramRecall));

      /* SKOTOS: Fetch the COOKIE string */

      browserWindow   = (JSObject) JSObject.getWindow(this);
      browserDocument = (JSObject) browserWindow.getMember("document");
      mapImage        = (JSObject) browserDocument.getMember("mapimg");

      if (mapImage == null) {
	 System.out.println("Found no mapImage!");
      } else {
	 System.out.println("Found an mapImage!");
      }

      Object Empty[] = { };

      /* Couldn't get win.eval('document.cookie') to work on IE */
      String cookie = (String) browserWindow.call("dacookie", Empty);
      charName = (String) browserWindow.call("daCharName", Empty);

      if (cookie != null) {
	 System.out.println("Got a cookie: " + cookie);

	 Hashtable cookBook = makeCookBook(cookie);

	 userName = (String) cookBook.get("user");
	 passWord = (String) cookBook.get("pass");
      } else {
	 System.out.println("Got no cookie.");
      }
      /* perform a MD5 just to preload the classes */

      new MD5().Update("Hello!");

      /* END SKOTOS */

   /*** Set everything up ***/

//*#debugstart*/ debug("init: creating recall buffer.");
      history = new String[paramRecall];
      Dimension mysize = size();

//*#debugstart*/ debug("init: creating input field.");
      /* Create the input field */
      inputWindow = new TextField(1);
      inputWindow.setEditable(true);
      inputWindow.setFont(paramFont);

//*#debugstart*/ debug("init: creating output window.");
      /* Create the output window */
      outputWindow = new TextArea("", 1, 1);
      outputWindow.setEditable(false);
      outputWindow.setFont(paramFont);

//*#debugstart*/ debug("init: determining font metrics.");
      /* Determine the size of the selected font and the output window's border height/width */
      FontMetrics fm = outputWindow.getFontMetrics(paramFont);
      charHeight = fm.getHeight();
//*#debugstart*/ debug("init: charHeight="+Integer.toString(charHeight));
      charWidth = fm.charWidth('H');
//*#debugstart*/ debug("init: charWidth="+Integer.toString(charWidth));
      Dimension owb = outputWindow.minimumSize();
      outWinBHeight = owb.height - charHeight;
//*#debugstart*/ debug("init: outWinBHeight="+Integer.toString(outWinBHeight));
      outWinBWidth = owb.width - charWidth;
//*#debugstart*/ debug("init: outWinBWidth="+Integer.toString(outWinBWidth));

      add(inputWindow);
      add(outputWindow);

      /* If "rows" or "cols" parameters were specified, adjust the base
       * applet size we work from to match what they need it to be
       * (based on the font size) */
      if (paramRows != 0) {
//*#debugstart*/ debug("init: adjusting to fit paramRows:");
        mysize.height = outWinBHeight + inputWindow.minimumSize().height
                        + charHeight * paramRows;
      }
      if (paramCols != 0) {
//*#debugstart*/ debug("init: adjusting to fit paramCols:");
        mysize.width = outWinBWidth + charWidth * paramCols;
      }
//*#debugstart*/ debug("init: adjusted size="+mysize);
      resize(mysize);

//*#debugstart*/ debug("init: setting colors.");
      /* Set all the colors up the way the applet parameters specified */
      if (paramBGcolor != null) setBackground(paramBGcolor);
      if (paramOBGcolor != null) outputWindow.setBackground(paramOBGcolor);
      if (paramOFGcolor != null) outputWindow.setForeground(paramOFGcolor);
      if (paramIFGcolor != null) {
	 inputWindow.setForeground(paramIFGcolor);
      }
      if (paramIBGcolor == null) paramIBGcolor = inputWindow.getBackground();

//*#debugstart*/ debug("init: disabling input field.");
      /* Gray-out the input field until we actually connect to somewhere */
      inputWindow.setEditable(false);
      inputWindow.setBackground(Color.lightGray);
//*#debugstart*/ debug("completed init()");

      System.err.println("init() exiting");
   }

   public void start() {
//*#debugstart || debugflow*/ debug("start() called.");
      if (remote != null) { remote.start(); return; }

      System.err.println("start() called");

      if ((netConnection == null) && (paramPort != 0)) {
//*#debugstart*/ debug("start: connection=null.  starting applet.");
         /* The page has been brought back onscreen and we're not
          * currently connected to anywhere.  Reset everything to
          * startup conditions and then try to reconnect. */
         int i;

         /* clear the command history */
//*#debugstart*/ debug("start: clearing command history.");
         history[0] = "";
         for (i=1; i < paramRecall; i++) {
            history[i] = null;
         }
         historyPos = 0;
	 
	 /* init the kill buffer */
	 killBuffer = "";

         /* zap all text in the input/output areas */
//*#debugstart*/ debug("start: clearing input/output text.");
         outputWindow.setText("");
         inputWindow.setText("");
         inputWindow.setEditable(false);
         inputWindow.setBackground(Color.lightGray);

         /* Put the cursor in the input field */
//*#debugstart*/ debug("start: requesting focus.");
         inputWindow.requestFocus();

         /* Start new connection thread */
//*#debugstart || debugt2*/ debug("start: creating secondary thread.");
         mainThread = new Thread(this);
//*#debugstart || debugt2*/ debug("start: starting secondary thread.");
         mainThread.start();
      }
//*#debugstart*/ debug("start() complete.");
      System.err.println("start() exiting");
   }

   public void stop() {
//*#debugevent*/ debug("stop() called.");
      if (remote != null) { remote.stop(); return; }
   }

   public void destroy() {
//*#debugevent*/ debug("destroy() called.");
      if (remote != null) { remote.destroy(); return; }

      /* If we're still connected make sure the net connection gets
       * closed cleanly */
      if (netConnection != null) {
//*#debugevent*/ debug("destroy: closing net connection.");
         try { netConnection.close(); } catch (IOException e) {}
      }
   }

   public void reshape(int x, int y, int width, int height) {
//*#debugevent*/ debug("reshape("+Integer.toString(x)+","+Integer.toString(y)+","+Integer.toString(width)+","+Integer.toString(height)+") called.");
      if (remote != null) { remote.reshape(x,y,width,height); return; }

      /* Calculate the width and height we _should_ be based on the
       * width and height we've been resized to and the horizontal and
       * vertical size corrections specified in the parameters (if any) */
      int newWidth = (int)((((float)width) / paramHcorrect[0])*100) + paramHcorrect[1];
      int newHeight = (int)((((float)height) / paramVcorrect[0])*100) + paramVcorrect[1];

      /* Make sure the size corrections wouldn't result in being chopped
       * off */
      if (newHeight > height) newHeight = height;
      if (newWidth > width) newWidth = width;

      /* Do the actual reshape with the corrected values.  Actually we
       * should be calling parent's reshape so that the whole applet
       * gets resized, but there's some strange stuff with the way
       * applet methods are called and recursion which I simply have not
       * been able to comprehend, so I gave up and just took the easy
       * way out.  I may fix this at some point if I ever do figure out
       * what's going on in the depths of Netscape's Java stuff (it's a
       * strange world in there) */
//*#debugevent*/ debug("reshape: calling super.reshape("+Integer.toString(x)+","+Integer.toString(y)+","+Integer.toString(newWidth)+","+Integer.toString(newHeight)+")");
      super.reshape(x, y, newWidth, newHeight);
   }

   /* layout() is called by the system when this component is
    * moved/resized to lay out its component objects.
    * Normally this would be handled by whatever LayoutManager we
    * associate with this component, but the default LayoutManagers
    * available are, frankly, crap, and defining a new one would involve
    * more overhead than it's worth.  We override this and do it
    * ourselves so we can be sure things get laid out as cleanly as
    * possible. */
   private int oldRows = 0;
   private int oldCols = 0;

   public void layout() {
//*#debugevent*/ debug("layout() called.");
      System.err.println("layout() called");

      if (remote != null) { remote.layout(); return; }

      if (inputWindow != null) {
//*#debugevent*/ debug("layout: inputWindow != null.  laying out components.");
         Dimension mysize = size();
//*#debugevent*/ debug("layout: mysize="+mysize);
         int cwheight = inputWindow.minimumSize().height;
         int owheight = mysize.height - cwheight;
         screenRows = (owheight - outWinBHeight) / charHeight - 2;
         screenCols = (mysize.width - outWinBWidth) / charWidth - 5;
//*#debugevent*/ debug("layout: resizing output window to "+Integer.toString(owheight));
         outputWindow.reshape(0, 0, mysize.width, owheight);
//*#debugevent*/ debug("layout: resizing input window to "+Integer.toString(cwheight));
         inputWindow.reshape(0, owheight, mysize.width, cwheight);
         if (screenRows != oldRows || screenCols != oldCols) {
//*#debugevent*/ debug("layout: rows/cols have changed.  re-sending window info.");
            oldRows = screenRows;
            oldCols = screenCols;
         }
      }
      System.err.println("layout() exiting");
   }

   /* We don't need to pass us this because any applet will get keyboard
    * events directly because we're a panel containing the input/output
    * window components.. */
   public boolean keyDown(Event evt, int key) {
       int k, l;

//*#debugevent || debugflow*/ debug("keyDown called.");
      /* User hit a key.. is the cursor in the input field? */
      if (evt.target == inputWindow) {
//*#debugevent*/ debug("keyDown: input window.");
         String command;

         switch (key) {

	 case 1: // ^A
	     System.out.println("Inside - " + key);
	     inputWindow.setCaretPosition(0);
	     break;
	 case 2: // ^B
	     System.out.println("Inside - " + key);
	     inputWindow.setCaretPosition(inputWindow.getCaretPosition() - 1);
	     break;
	 case 4: // ^D
	     System.out.println("Inside3 - " + key);
	     k = inputWindow.getCaretPosition();
	     command = inputWindow.getText();
	     inputWindow.setText(command.substring(0, k) + command.substring(k+1, command.length()));
	     inputWindow.setCaretPosition(k);
	     break;
	 case 5: // ^E
	     System.out.println("Inside - " + key);
	     inputWindow.setCaretPosition(inputWindow.getText().length());
	     break;
	 case 6: // ^F
	     System.out.println("Inside - " + key);
	     inputWindow.setCaretPosition(inputWindow.getCaretPosition() + 1);
	     break;
	 case 11: // ^K
	     System.out.println("Inside4 - " + key);
	     k = inputWindow.getCaretPosition();
	     command = inputWindow.getText();
	     killBuffer = command.substring(k, command.length());
	     inputWindow.setText(command.substring(0, k));
	     break;
	 case 23: // ^W backdelete word.
	     System.out.println("Inside4 - " + key);
	     k = inputWindow.getCaretPosition();
	     command = inputWindow.getText();
	     l = command.lastIndexOf(" ", k);
	     
	     killBuffer = command.substring(l, k);
	     
	     // (delete from the current carret point to the last white space)
	     inputWindow.setText(command.substring(0, l) + command.substring(k, command.length()));
	     break;
	 case 25: // ^y yank from buffer
	     System.out.println("Inside5 - " + key);
	     k = inputWindow.getCaretPosition();
	     command = inputWindow.getText();
	     inputWindow.setText(command.substring(0, k) + killBuffer + command.substring(k, command.length()));
	     inputWindow.setCaretPosition(k + killBuffer.length());
	     break;
	     
          case 14:    // ^n
          case 16:    // ^p
            command = inputWindow.getText();
            if (!command.equals(history[historyPos])) history[0] = command;
            historyPos += (key == 14) ? 1 : -1;
            if (historyPos < 0) historyPos++;
            if (historyPos >= paramRecall || history[historyPos] == null) historyPos--;
            inputWindow.setText(history[historyPos]);
            break;

	 case '\n':        // Enter key.  Send the line.
	     //*#debugevent*/ debug("keyDown: enter key.");
	     command = inputWindow.getText();
//*#debugevent*/ debug("keyDown: non-password mode.");
	    if (command.length() != 0) {
//*#debugevent*/ debug("keyDown: non-empty line. adding to history.");
	       for (int i=paramRecall-1; i > 1; i--)
		  history[i] = history[i-1];
	       history[1] = command;
	       history[0] = "";

               if (paramEcho != null) {
//*#debugevent*/ debug("keyDown: echoing to output window.");
                  outputText(paramEcho+command+"\n");
               }
            }
//*#debugevent*/ debug("keyDown: sending: "+command);
            netWrite(netConnection, command+"\r\n");
//*#debugevent*/ debug("keyDown: clearing input field.");
            inputWindow.setText("");
            break;
          case '\t':        // Tab key.  Switch to the output window.
//*#debugevent*/ debug("keyDown: tab key.  switching focus to output window.");
            outputWindow.requestFocus();
            break;
          case Event.UP:    // Up/Down arrows.
          case Event.DOWN:  //   Scroll through command history.
//*#debugevent*/ debug("keyDown: up/down key.");
            command = inputWindow.getText();
            if (!command.equals(history[historyPos])) history[0] = command;
            historyPos += (key == Event.UP) ? 1 : -1;
            if (historyPos < 0) historyPos++;
            if (historyPos >= paramRecall || history[historyPos] == null) historyPos--;
//*#debugevent*/ debug("keyDown: new history position="+Integer.toString(historyPos));
            inputWindow.setText(history[historyPos]);
            break;
          default:          // Anything else, let the input field deal
//*#debugevent*/ debug("keyDown: non-special key.");
	     return false;   // with it normally.
         }
         return true;
      } else if (evt.target == outputWindow) {
//*#debugevent*/ debug("keyDown: output window.");
         /* Cursor was in the output window when the key was hit.
          * Moving around and selecting text with the cursor keys in the
          * output window is ok, but if the user tries to type some
          * text, it should go into the input window instead. */
         /* (the following isn't quite kosher as it assumes things about
          * the ordering of the Event.* key numbers, but it should work
          * ok and it's a little smaller/quicker than testing each
          * possibility individually) */
         if (key < Event.HOME || key > Event.RIGHT) {
            /* User tried to type something while in the output window.
             * Switch the focus to the input window and give it the
             * keystroke instead. */
//*#debugevent*/ debug("keyDown: non-movement key.  changing focus to input window.");
            inputWindow.requestFocus();
            if (key > 31 && key < 127 && inputWindow.isEditable()) {
//*#debugevent*/ debug("keyDown: adding keystroke to input field.");
               /* Grumble.. this is the best we can do because the AWT
                * classes don't provide a proper two-way wrapper like
                * they should (so we can't send a keyDown event to a
                * TextField and have it actually do anything). */

               String cmdtext = inputWindow.getText();
               int curpos = inputWindow.getSelectionEnd();
               inputWindow.setText(cmdtext.substring(0,curpos)
                               + (new Character((char)key)).toString()
                               + cmdtext.substring(curpos));
               inputWindow.select(curpos+1, curpos+1);
            }
            return true;
         }
      }
      return false;
   }

   /* The run() method is called by mainThread when it starts up.  When
    * this happens we try to connect to the host and then handle the
    * read loop in this thread. */
   public void run() {
      String data;

//*#debugflow*/ debug("run() called.");
      /* try to connect.. */
      showStatus("Connecting...");

      try {
//*#debugio || debugt2*/ debug("run: creating socket.");
	  //         netConnection = new Socket(paramHost, paramPort);
      } catch (Exception e) {
         log("Exception on socket open: "+e);
         showStatus("Connection to "+paramName+" Failed!");
         return;
      }
      /* we connected ok.  Set up the input window properly and say
       * we've connected.. */
//*#debugt2*/ debug("run: setting input field editable.");

      inputWindow.setEditable(true);
      inputWindow.setBackground(paramIBGcolor);
      showStatus("Connected to "+paramName+".");

      if (userName != null && passWord != null) {
	 netWrite(netConnection, "SKOTOS\n");
      } else {
	 System.out.println("Help! Lacking userName or passWord!");
      }	    

      /* If a "command" parameter was specified, send the initial
       * command out.. */
      if (paramCommand != null) {
//*#debugio || debugt2*/ debug("run: sending: "+paramCommand);
         netWrite(netConnection, paramCommand+"\r\n");
      }

      /* Do a read loop until the connection has been closed. */
//*#debugt2*/ debug("run: entering read loop.");
      while (netConnection != null &&
	     (data = netRead(netConnection)) != null) {
//*#debugio*/ debug("run: received \""+data+"\"");
         outputText(processText(data));
      }
//*#debugt2*/ debug("run: finished read loop.");

      /* Connection has closed.  Do cleanup and disable input. */
      netConnection = null;
//*#debugt2*/ debug("run: disabling input field.");
      inputWindow.setEditable(false);
      inputWindow.setBackground(Color.lightGray);
      showStatus("Connection closed.");
//*#debugt2*/ debug("run() completed.");
   }

// ====================== MUVE-Output Handling Methods =======================

   /* Some constants for the TELNET-handling routines: */
   private static final int IAC  = 255;
   private static final int WILL = 251;
   private static final int WONT = 252;
   private static final int DO   = 253;
   private static final int DONT = 254;
   private static final int ECHO = 1;

   /* Variables for things in this section: */
   private String fragmentBuffer = "";
   private boolean prevNewline = true;
   private byte[] IACbuf = {(byte)IAC, (byte)0, (byte)0};
   private String MCPauthKey = null;

   /* processText(String) - Perform all filtering and pre-processing
    * (TELNET, MCP, etc) on text received from the MUVE. This is a
    * little complicated because we may receive text in blocks which
    * have nothing to do with where the lines end.  We have to handle
    * single method calls which contain text for multiple lines,
    * lines which span multiple calls to this method, or both at the
    * same time. */
   /* NOTE: Due to its state-dependent nature, this method can be used
    *       by a SINGLE-THREAD ONLY.  In the case of this applet it is
    *       only called by the connection-reading thread. */
   private String processText(String text) {
      int start, end;
      String outText = "";
      int IACpos;
      int skip;
//*#debugflow*/ debug("ProcessText called.");
      /* fragmentBuffer holds any text left over from the end of the
       * last call which may be important for parsing this one.  If we
       * have a line-fragment left over, tack it onto the beginning of
       * this one and process it as a whole. */
      text = fragmentBuffer + text;
      fragmentBuffer = "";
//*#debugtel || debugio*/ debug("ProcessText: processing IAC sequences...");
      /* Process TELNET IAC sequences. */
      while ((IACpos = text.indexOf(IAC)) != -1) {
         try {
            skip = 3;
            /* What type of IAC is it? */
            int nextchar = text.charAt(IACpos+1);
            switch (nextchar) {
             case DO:
             case DONT:
//*#debugtel*/ debug("ProcessText: DO/DONT received.");
               /* We don't recognize any DO/DONT requests.  Respond with
                * a negative (WONT) response. */
               IACbuf[1] = (byte)WONT;
               IACbuf[2] = (byte)text.charAt(IACpos+2);
	       if (netConnection != null) {
		  netWrite(netConnection, IACbuf);
	       }
               break;
             case WILL:
//*#debugtel*/ debug("ProcessText: WILL received:");
               IACbuf[2] = (byte)text.charAt(IACpos+2);
//*#debugtel*/ debug("ProcessText: non-ECHO request.  Responding with DONT response."); 
	       /* We don't handle anything besides ECHO, so tell it
		* not to do whatever it wants to do. */
	       IACbuf[1] = (byte)DONT;
               /* Send the response. */
//*#debugtel*/ debug("ProcessText: Sending IAC response.");
	       if (netConnection != null) {
		  netWrite(netConnection, IACbuf);
	       }
               break;
             case WONT:
//*#debugtel*/ debug("ProcessText: WONT received.");
               break;
             case IAC:
//*#debugtel*/ debug("ProcessText: IAC IAC received.");
               /* We got an "IAC IAC" (an escaped IAC code).  Put a
                * plain IAC (ASCII 255) in its place. (actually, we just
                * leave the first IAC there and skip the second one) */
               IACpos += 1;
               skip = 1;
               break;
             default:
//*#debugtel*/ debug("ProcessText: unrecognized IAC sequence.  skipping two characters.");
               /* Unrecognized IAC sequence.  Since all sequences other
                * than DO/DONT/WILL/WONT are always 2-character
                * sequences, we'll just skip these two characters and
                * continue on. */
               skip = 2;
            }
            /* Move the appropriate bits of the input text to the output
             * text and then loop. */
//*#debugtel*/ debug("ProcessText: copying "+Integer.toString(IACpos)+" and skipping "+Integer.toString(skip));
            outText = outText + text.substring(0, IACpos);
            text = text.substring(IACpos + skip);
         } catch (StringIndexOutOfBoundsException e) {
//*#debugtel*/ debug("ProcessText: index out of bounds.  copying "+Integer.toString(IACpos)+" and saving rest for next call.");
            /* We tried to access beyond the end of the string
             * somewhere, indicating we have a partial IAC sequence, and
             * we need to store what we have so far in the
             * fragmentBuffer for processing when the rest of it comes
             * in later. */
            outText = outText + text.substring(0, IACpos);
            fragmentBuffer = text.substring(IACpos) + fragmentBuffer;
            text = "";
         }
      }
      /* Done with TELNET processing, make the results the input to the
       * next stage of processing. */
      text = outText + text;
//*#debugtel || debugio*/ debug("ProcessText: completed TELNET processing.");

      /* Process MCP request lines. */
      if (paramMCP) {
//*#debugmcp || debugio*/ debug("ProcessText: processing MCP requests.");
         outText = "";
         /* If the previous block of text ended in a newline, put a
          * newline on the beginning of this to make sure that the
          * following search will match correctly. */
         if (prevNewline) {
//*#debugmcp*/ debug("ProcessText: previous newline.  prepending \\n.");
            text = "\n"+text;
         }
         /* Look for lines beginning with "#$#".. */
         while ((start = (text).indexOf("\n#$#")) != -1) {
//*#debugmcp*/ debug("ProcessText: found #$# line.  copying "+Integer.toString(start));
            /* Stick any preceeding text into the output unmodified. */
            outText = outText + text.substring(0, start);
            /* Look for the end of the #$#-line. */
            end = text.indexOf('\n', start+1);
            if (end == -1) {
//*#debugmcp*/ debug("ProcessText: end of line not found.  saving for next call.");
               /* The end of the line isn't in this block.. put the line
                * in the line-fragment-buffer and wait for it to be
                * completed on a future call. */
               fragmentBuffer = text.substring(start) + fragmentBuffer;
               text = "";
            } else {
//*#debugmcp*/ debug("ProcessText: MCP line isolated: "+text.substring(start+1, end));
//*#debugmcp*/ debug("ProcessText: calling doMCPrequest.");
               /* We've isolated the #$#-line.  Pass it to doMCPrequest
                * and put any results into the output. */
               outText = outText + doMCPrequest(text.substring(start+1, end));
               text = text.substring(end);
            }
         }
         text = outText + text;
         if (prevNewline) {
//*#debugmcp*/ debug("ProcessText: removing prepended \\n.");
            /* We tacked on an \n above, so undo that. */
            text = text.substring(1);
         }
         /* If this block ended with a newline, then set prevNewline for
          * the next call. */
         prevNewline = text.equals("") ? prevNewline : text.endsWith("\n");
//*#debugmcp || debugio*/ debug("ProcessText: completed MCP processing.");
      }

      /* If the "waitfor" parameter was specified, then we'll just eat
       * any output text until we find the text we're looking for.
       * We have to do this after all the other processing so that
       * telnet sequences are filtered out, and so we don't miss
       * important MCP sequences or match text in an MCP sequence which
       * we shouldn't be matching. */
      if (paramWaitfor != null) {
//*#debugio*/ debug("ProcessText: searching for waitfor text.");
        /* Did we find it? */
        if ((start = text.indexOf(paramWaitfor)) == -1) {
//*#debugio*/ debug("ProcessText: text not found.  eating all remaining text.");
          return "";
        } else {
//*#debugio*/ debug("ProcessText: text found.");
          /* Yes! Chop off any text before the line with the matching
           * bit in it, and then zap paramWaitfor so that we won't keep
           * looking anymore. */
          text = text.substring(text.lastIndexOf('\n',start)+1);
          paramWaitfor = null;
        }
      }
      /* SKOTOS */
      if ((start = text.indexOf("MAPURL ")) != -1) {
	 int nlpos = text.indexOf("\n", start);
	 String URL = text.substring(start + 7, nlpos);

	 System.out.println(text);
	 System.out.println("Start: " + start);
	 System.out.println("NLPos: " + nlpos);

	 mapImage.setMember("src", URL);

	 if (start > 0) {
	    text = text.substring(0, start) + text.substring(nlpos+1);
	 } else {
	    text = text.substring(nlpos+1);
	 }
      } else if ((start = text.indexOf("SECRET ")) != -1) {
	 if (userName == null || passWord == null) {
	    System.out.println("Help! I lack userName or passWord!");
	    return text;
	 }
	 if (netConnection == null) {
	    System.out.println("Connection is closed -- not authentication.");
	 }
	 int nlpos = text.indexOf("\n", start);
	 String secret = text.substring(start + 7, nlpos);
	 System.out.println("Got server secret: " + secret);

	 if (start > 0) {
	    text = text.substring(0, start-1) + text.substring(nlpos+1);
	 } else {
	    text = text.substring(nlpos+1);
	 }
	 netWrite(netConnection, "USER " + userName + "\n");
	 netWrite(netConnection, "SECRET " + secret + "\n");

	 MD5 hashVal = new MD5();
	 hashVal.Update(userName);
	 hashVal.Update(passWord);
	 hashVal.Update(secret);

	 System.out.println("Got a hash: " + hashVal.asHex());

	 netWrite(netConnection, "HASH " + hashVal.asHex() + "\n");

	 netWrite(netConnection, "CHAR " + charName + "\n");
      }
      /* END SKOTOS */
//*#debugio*/ debug("returning from ProcessText: "+text);
      return text;
   }

   /* doMCPrequest(String) - Take a line containing an MCP/1.0-style
    * request and do whatever's appropriate with it.  Returns text (if
    * any) which should be displayed in the output window in place of
    * the request. */
   private String doMCPrequest(String request) {
      String type, key = "";
      String[] parseresult;
      Hashtable parameters = new Hashtable(5);
//*#debugflow*/ debug("doMCPrequest called.");

      /* Break the line up into its component parts... */
      String line = request + " ";
      /* Skip the #$# at the beginning, and take everything up to the
       * first space as the request type */
      int space = line.indexOf(' ');
      type = line.substring(3, space);
      line = line.substring(space+1);
      /* Look at the next word to see if it's an authentication key or
       * the beginning of the key/value data */
//*#debugmcp*/ debug("doMCPrequest: type="+type);
      parseresult = parseWord(line);
      if (!parseresult[0].endsWith(":")) {
         key = parseresult[0];
         line = parseresult[1];
//*#debugmcp*/ debug("key="+key);
      }
      
      /* For the rest of the line, split it up by words and stick the
       * key/value parameter data into a Hashtable... */
      while (!line.equals("")) {
        parseresult = parseWord(line);
        if (parseresult[0].endsWith(":")) {
           String paramkey = parseresult[0];
           parseresult = parseWord(parseresult[1]);
           parameters.put(paramkey, parseresult[0]);
//*#debugmcp*/ debug("doMCPrequest: param "+paramkey+"="+parseresult[0]);
        }
        line = parseresult[1];
      }

      /* Now check the type of the request and do whatever's
       * appropriate. */
      if (type.equalsIgnoreCase("mcp")) {
//*#debugmcp*/ debug("doMCPrequest: #$#mcp request.");
         /* #$#mcp opening announcement.  Respond with authentication
          * key initialization and other status info. */
         mcp_enabled = true;
         MCPauthKey = String.valueOf(Math.random());
//*#debugmcp*/ debug("doMCPrequest: sending authentication key.");
         netWrite(netConnection, "#$#authentication-key "+MCPauthKey+"\r\n");
//*#debugmcp*/ debug("doMCPrequest: returning \"\".");
         return "";
      }

//*#debugmcp*/ debug("doMCPrequest: checking authentication key.");
      if ((MCPauthKey != null || paramMCPauth) && !key.equals(MCPauthKey)) {
         /* Everything except an #$#mcp announcement requires that the
          * key be correct. */
         log("Bad/missing authentication key for MCP request:\n  "+request);
         return "";
      }


      if (type.equals("display-url")) {
         /* #$#display-url request.  Make the browser load the specified
          * url ('url') in the specified window ('target'). */
         String url, target;
//*#debugmcp*/ debug("doMCPrequest: #$#display-url request.");

         if ((url = (String)parameters.get("url:")) == null) {
            /* All #$#display-url requests must have a "url" parameter */
            log("Malformed MCP display-url request:\n  "+request);
            return "";
         }
         if ((target = (String)parameters.get("target:")) == null) {
//*#debugmcp*/ debug("doMCPrequest: no target specified.  using urltarget applet parameter.");
            /* request didn't specify a target.  Use whatever was
             * specified in the "urltarget" parameter for the applet, if
             * one was. */
            target = paramURLtarget;
         }
         /* Try to display the requested URL.. */
         try {
            if (target == null) {
//*#debugmcp*/ debug("doMCPrequest: calling showDocument("+url+")");
               getAppletContext().showDocument(new URL(url));
            } else {
//*#debugmcp*/ debug("doMCPrequest: calling showDocument("+url+", "+target+")");
               getAppletContext().showDocument(new URL(url), target);
            }
         } catch (MalformedURLException e) {
            log("Malformed URL for MCP display-url request:\n  "+request);
         }
//*#debugmcp*/ debug("doMCPrequest: returning \"\".");
         return "";
      }

      /* Not a #$#mcp or #$#display-url request.. we don't know what to
       * do with it. */
      log("Unrecognized MCP request:\n  "+request);
      return "";
   }

   private boolean doNewline = false;

   private void outputText(String text) {
//*#debugflow*/ debug("outputText called.");
      paramBackscroll -= text.length();
      if (paramBackscroll < -BACKSCROLL_OVERDRAFT) {
//*#debugout*/ debug("outputText: cleaning backscroll.");
         String temp = outputWindow.getText();
         outputWindow.setText("");
         outputWindow.appendText(temp.substring(-paramBackscroll));
         paramBackscroll = 0;
      }
      if (doNewline) {
//*#debugout*/ debug("outputText: previous call ended with newline.  prepending \\n.");
         text = "\n"+text;
      }
      if (text.endsWith("\n")) {
//*#debugout*/ debug("outputText: ends with \\n.  setting doNewline.");
         doNewline = true;
      } else {
//*#debugout*/ debug("outputText: doesn't end with \\n.  appending \\n.");
         doNewline = false;
         text = text + "\n";
      }
      int pos, start = 0;
      boolean linePartial = true;
      while ((pos = text.indexOf("\n",start)) != -1) {
         if (start != 0) {
//*#debugout*/ debug("outputText: adding end of line.");
            outputWindow.appendText("\n");
         }
         if (pos == start) {
//*#debugout*/ debug("outputText: blank line.  no wrapping.");
         } else if (linePartial) {
//*#debugout*/ debug("outputText: previous partial line.  prepending and re-wrapping.");
            String outputText = outputWindow.getText();
            int linePartialStart = outputText.lastIndexOf("\n")+1;
            String addText = wrapLine(outputText.substring(linePartialStart) + text.substring(start, pos));
            outputWindow.replaceText(addText, linePartialStart, outputText.length());
         } else {
//*#debugout*/ debug("outputText: wrapping text and appending to output window.");
            outputWindow.appendText(wrapLine(text.substring(start, pos)));
         }
         linePartial = false;
         start = pos+1;
      }

      while ((pos = outputWindow.getText().indexOf("\010")) != -1) {
	 System.out.println("OK, find one at " + pos);
	 if (pos > 0) {
	    outputWindow.replaceText("", pos-1, pos+1);
	 } else {
	    outputWindow.replaceText("", 0, 1);
	 }
      }

//*#debugout*/ debug("outputText: placing output cursor at bottom of window.");
      outputWindow.select(pos = outputWindow.getText().length(), pos);
   }

   private String wrapLine(String line) {
      String output = "";

//*#debugflow*/ debug("wrapLine called.");
      while (line.length() > screenCols) {
         int splitpos = line.lastIndexOf(" ",screenCols);
         if ((splitpos != -1) && (screenCols - splitpos <= WRAP_MARGIN)) {
//*#debugwrap*/ debug("wrapLine: word-wrapping at "+Integer.toString(splitpos));
           output = output + line.substring(0, splitpos) + "\n";
           line = line.substring(splitpos+1);
         } else {
//*#debugwrap*/ debug("wrapLine: splitting at "+Integer.toString(screenCols));
           output = output + line.substring(0, screenCols) + "\n";
           line = line.substring(screenCols);
         }
      }
      return output + line;
   }

// ========================== Basic Utility Methods ==========================

   /* log(String) - log a message on the Java Console, prefixed by the
    * name of the MUVE we're currently connected to. */
   private void log(String message) {
      System.out.println(paramName+": "+message);
   }

   /* netBuffer is really only used by netRead, but we define it as a
    * class member variable so we don't have to waste time
    * allocating/deallocating it every time netRead is called. */
   private byte netBuffer[] = null;

   /* netRead(Socket) - Read any waiting input from the specified
    * socket.  Returns a String containing the waiting input, if any, or
    * null if it was unable to read from the socket (connection's been
    * closed).  Automatically converts CRLFs to newlines, but the read
    * text is NOT broken into individual lines.  A given block of text
    * returned from this method may contain multiple lines, partial
    * lines, or both. */
   private String netRead(Socket s) {
      int len;
      int i, j;

//*#debugflow*/ debug("netRead called.");
      /* Check to make sure the buffer has been allocated */
      if (netBuffer == null)
         netBuffer = new byte[NET_BUFFER_SIZE];

      /* Read any waiting input for the connection... */
      try {
//*#debugio*/ debug("netRead: waiting for net input.");
         len = s.getInputStream().read(netBuffer);
      } catch (IOException e) {
         /* read() threw an error.  This shouldn't normally happen, so
          * we have to assume there's something wrong with the
          * connection and just abandon it. */
         log("Exception on socket read: "+e);
         try { s.close(); } catch (IOException e1) {}
         /* pretend we got a -1 (connection closed) from the read. */
         len = -1;
      }
//*#debugio*/ debug("netRead: received "+Integer.toString(len)+" bytes.");
      if (len == -1) {
//*#debugio*/ debug("netRead: connection closed.");
         /* connection closed. */
         return null;
      }
      /* Go through the block of text we got and filter it
       * appropriately. */
      j=0; for (i=0; i < len; ) {
         /* ignore CRs. */
         if (netBuffer[i] == '\r')
            i++;
         else
            netBuffer[j++] = netBuffer[i++];
      }
      /* Convert the buffer into a String and return it. */
//*#debugio*/ debug("netRead: returning read data.");
      return new String(netBuffer, 0, 0, j);
   }

   /* netWrite(Socket, String) - Write the specified text to the socket.
    * Does NOT perform any processing on the output (CRLFs must be
    * provided as \r\n in the string, for example.  \n will only send a LF
    * character).
    * This actually just converts the String to a byte[] buffer and
    * passes it to netWrite(Socket, byte[]).
    * Returns true if output was written, false if it was not. */
   private boolean netWrite(Socket s, String string) {
//*#debugflow*/ debug("netWrite(string) called.");
      if (s == null) {
//*#debugio*/ debug("netWrite(string): null string.");
         return false;
      }

//*#debugio*/ debug("netWrite(string): converting to buffer.");
      int len = string.length();
      byte buffer[] = new byte[len];
      string.getBytes(0, len, buffer, 0);
//*#debugio*/ debug("netWrite(string): calling netWrite(buffer)");
      return netWrite(s, buffer);
   }

   /* netWrite(Socket, byte[]) - Write the specified text to the socket.
    * Does NOT perform any processing on the output (CRLFs must be
    * provided as {'\r','\n'} in the buffer, for example.  '\n' will
    * only send a LF character).
    * Returns true if output was written, false if there was a problem. */
   private boolean netWrite(Socket s, byte[] buffer) {
//*#debugio*/ debug("netWrite(buffer) called.");

      try {
//*#debugio*/ debug("netWrite: writing buffer to socket.");
         s.getOutputStream().write(buffer);
         return true;
      } catch (IOException e) {
         /* write() threw an error.  This shouldn't normally happen, so
          * we have to assume there's something wrong with the
          * connection and just abandon it. */
         log("Exception on socket write: "+e);
         try { netConnection.close(); } catch (IOException e1) {}
	 netConnection = null;	/* SKOTOS */
         return false;
      }
   }

   /* parseWord(String) - Break the specified string down based on "word"
    * boundaries.  This function will split the string into its first
    * word and the rest of the string, and return the result as an array
    * of two strings.  Multiple spaces are treated as a single word-
    * break, and portions enclosed in quotes are considered single
    * words.  This is consistent with the parsing requirements for MCP.
    * Backslash-quote and backslash-backslash can be used to escape
    * quotes and backslashes within a quoted portion, but outside of a
    * quoted portion they are not translated properly.  This is ok for
    * MCP as the MCP spec requires that words be quoted if they contain
    * quote characters, etc, but it's not intuitive for most other
    * applications.
    * This routine also has the side-effect of trimming off any whitespace at
    * the end of the string and adding a single space.
    * Basically, this routine works for MCP parsing but may need
    * tweaking if one wants to use it in a different application. */
   private String[] parseWord(String string) {
      int space, quote, escquote, escesc;
      String parsed = "";
      String[] result = new String[2];

//*#debugflow*/ debug("parseWord called.");
      string = string.trim() + " ";
      while (true) {
         /* Loop through quoted/unquoted bits of the string until we
          * come to an unquoted space. */
         space = string.indexOf(' ');
         quote = string.indexOf('"');
         if (space < quote || quote == -1) {
            /* The next word is a plain word with no quotes, so just
             * split it at the space and return the result. */
            result[0] = parsed + string.substring(0, space);
            result[1] = string.substring(space).trim();
            return result;
         } else {
            /* sigh.. quoted portion to contend with.. */
            parsed = string.substring(0, quote);
            string = string.substring(quote);
            while (true) {
               /* Look for the next "special sequence" in the string
                * (either a close-quote, escaped quote (backslash-quote),
                * or an escaped escape (backslash-backslash)), and deal
                * with it appropriately.  Keep going until we find the
                * close-quote or the end of the string. */
               quote = string.indexOf('"');
               escquote = string.indexOf("\\\"");
               escesc = string.indexOf("\\\\");
               if (escquote != -1
                   && (quote  == -1 || escquote < quote)
                   && (escesc == -1 || escquote < escesc)) {
                  /* the backslash-quote is the next special sequence
                   * in the string. */
                  parsed = parsed + string.substring(0, escquote)
                         + "\"";
                  string = string.substring(escquote+2);
               } else if (escesc != -1
                   && (quote == -1 || escesc < quote)) {
                  /* the backslash-backslash is the next special
                   * sequence in the string. */
                  parsed = parsed + string.substring(0, escquote)
                         + "\\";
                  string = string.substring(escquote+2);
               } else if (quote != -1) {
                  /* the close-quote is the next special sequence in the
                   * string. */
                  parsed = parsed + string.substring(0, quote);
                  string = string.substring(quote+1);
                  break;
               } else {
                  /* no special sequences left (there should've been a
                   * close-quote, but deal with it gracefully anyway).
                   */
                  result[0] = parsed + string;
                  result[1] = "";
                  return result;
               }
            }
         }
      }
   }

// ======================= Parameter Parsing Functions =======================

   /* parseBool(String, boolean) - Parse the given string, assuming it's
    * a boolean (yes/no) specification of some kind.  Returns true or
    * false depending on the provided string, or the specified default
    * (defvalue) if the string == null. */
   private boolean parseBool(String string, boolean defvalue) {
      if (string == null)
         return defvalue;

      /* We consider "yes", "y", "true", "on", or "1" (any case) to be
       * 'true' values, and anything else to be false. */
      if (string.equalsIgnoreCase("yes")  || string.equalsIgnoreCase("y")
       || string.equalsIgnoreCase("true") || string.equalsIgnoreCase("on")
       || string.equals("1"))
         return true;
      else
         return false;
   }

   /* parseInt(String, int) - Parse the given string, assuming it
    * specifies an integer value, and return the resulting integer.
    * Returns the supplied default value if the string is an invalid
    * form for an integer. */
   private int parseInt(String string, int defvalue) {
      try { return Integer.parseInt(string); }
      catch (NumberFormatException e) { return defvalue; }
   }

   /* parseFont(String, Font) - Parse the given string, assuming it is a
    * font specifier of the form '<fontname>[-<style>][.<pointsize>]'.
    * Default name, style, and size information is taken from the
    * specified default value. */
   private Font parseFont(String name, Font defvalue) {
      if (name == null)
         return defvalue;

      int pos;
      /* Get the default size and style */
      int size = defvalue.getSize();
      int style = defvalue.getStyle();

      /* If the string ends with ".<number>", take that as the size. */
      try {
         if ((pos = name.lastIndexOf('.')) != -1) {
            size = Integer.parseInt(name.substring(pos+1));
            name = name.substring(0, pos);
         }
      } catch (Exception e) {}
      /* If the fontname ends with "-<style>", use that as the style. */
      try {
         if ((pos = name.lastIndexOf('-')) != -1) {
            String suffix = name.substring(pos+1);
            if (suffix.equalsIgnoreCase("bold")) {
               style = Font.BOLD;
            } else if (suffix.equalsIgnoreCase("italic")) {
               style = Font.ITALIC;
            } else if (suffix.equalsIgnoreCase("bolditalic")) {
               style = Font.BOLD | Font.ITALIC;
            } else {
               style = Font.PLAIN;
            }
            name = name.substring(0, pos);
         }
      } catch (Exception e) {}
      /* If no font name was specified, use the one from defvalue. */
      if (name.equals(""))
         name = defvalue.getName();

      /* Create a font with the specified parameters and return it. */
      return new Font(name, style, size);
   }

   /* parseColor(String, Color) - Parse the given string, assuming it is
    * a color specification of the form 'RRGGBB' (where R, G, and B are
    * hex digits), '#RRGGBB', or '0xRRGGBB'.  Returns the specified
    * default value if it was unable to parse the string. */
   private Color parseColor(String string, Color defvalue) {
      try {
         if (string.startsWith("#"))
            /* strip the leading "#".. */
            string = string.substring(1);
         else if (string.startsWith("0x"))
            /* strip the leading "0x".. */
            string = string.substring(2);
         /* Parse it as a hex integer and create a Color based on the
          * resulting number */
         return new Color(Integer.parseInt(string, 16));
      } catch (Exception e) { return defvalue; }
   }

   /* parseCorrect(String, int[]) - Parse the given string, assuming it
    * is a correction specifier of the form '<int>,<int>', and returns
    * the two integers or the default set of integers if it was unable
    * to parse the string.  See the Cup-O MUD docs for a more detailed
    * description of what correction specifications mean and how they
    * work. */
   private int[] parseCorrect(String string, int[] defvalue) {
      try {
          int[] parsed = new int[2];
          int comma = string.indexOf(",");
          parsed[0] = Integer.parseInt(string.substring(0, comma));
          parsed[1] = Integer.parseInt(string.substring(comma+1));
          return parsed;
      }
      catch (Exception e) { return defvalue; }
   }

// ============================= Debugging Stuff =============================

//*#debug*/ private void debug(String message) {
//*#debug*/    System.out.println("CupOmud: "+message);
//*#debug*/ }

// ============================ That's All Folks! ============================


private String stripString(String str) {
   /* the brutally ugly way */
   while (str.length() > 0 && str.charAt(0) == ' ') {
      str = str.substring(1);
   }
   while (str.length() > 0 && str.charAt(str.length()-1) == ' ') {
      str = str.substring(0, str.length()-2);
   }
   return str;
}

public Hashtable makeCookBook(String cookie) {
   Hashtable cookBook = new Hashtable(5);
   String var, val;
   int sz = cookie.length();
   int ix = 0;
   int wix;

   do {
      wix = ix;

      while (ix < sz && cookie.charAt(ix) != '=') {
	 ix ++;
      }
      if (ix == sz) {
	 /* this entry ended mid-cookie; ignore it */
	 return cookBook;
      }
      if (ix > wix) {
	 var = stripString(cookie.substring(wix, ix));
      } else {
	 /* the '=' was found without anything in front of it */
	 var = null;
      }
      ix ++;
      wix = ix;

      while (ix < sz && cookie.charAt(ix) != ';') {
	 ix ++;
      }
      if (var != null) {
	 if (ix > wix) {
	    val = stripString(cookie.substring(wix, ix));
	    cookBook.put(var, val);
	    System.out.println("Putting " + var + " = " + val);
	 } else {
	    cookBook.put(var, "");
	 }
      }
      ix ++;
   } while (ix < sz);

   return cookBook;
}

public void writeIntoInputWindow(String str) {
   /* doubleclick: if this is precisely what is there already, send it */
   if (str.equalsIgnoreCase(inputWindow.getText())) {
      /* this is a copy from above */
      for (int i=paramRecall-1; i > 1; i--)
	 history[i] = history[i-1];
      history[1] = str;
      history[0] = "";

      if (paramEcho != null) {
	 outputText(paramEcho+str+"\n");
      }
      netWrite(netConnection, str+"\r\n");
      inputWindow.setText("");
      return;
   }
   /* else just write the text into it */
   inputWindow.setText(str);
}

}
