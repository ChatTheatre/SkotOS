/*
 * This code is based on an example provided by Richard Stanford, 
 * a tutorial reader.
 */

import java.awt.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;

import java.applet.*;
import java.util.*;
import java.io.*;
import java.net.*;
import java.lang.*;

public class TreeOfWoe extends JApplet implements Runnable {
private WoeNode			rootNode;
private DefaultTreeModel	treeModel;
private JTree			tree;

private Hashtable		nodeTable;
private Thread			mainThread;

private String			hostName;
private int			portBase;


private class WoeNode extends DefaultMutableTreeNode {
private String fullPath;
private URL clickURL;

public WoeNode(Object user, String path, URL URL) {
    super(user);
    fullPath = path;
    clickURL = URL;
}

public WoeNode(Object user) {
    this(user, null, null);
}

public WoeNode(Object user, String path) {
    this(user, path, null);
}

public String getFullPath() { return fullPath; }
public URL    getClickURL() { return clickURL; }
}


public TreeOfWoe() {
    try {
	UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
    } catch (Exception e) {}

    rootNode = new WoeNode("Root Node");
    treeModel = new DefaultTreeModel(rootNode);
    /* treeModel.addTreeModelListener(new MyTreeModelListener()); */

    tree = new JTree(treeModel);
    tree.setEditable(false);
    tree.setRootVisible(true);
    tree.setShowsRootHandles(false);
    tree.getSelectionModel().setSelectionMode
	(TreeSelectionModel.SINGLE_TREE_SELECTION);

    nodeTable = new Hashtable();
    nodeTable.put("", rootNode);

//Listen for when the selection changes.
    tree.addTreeSelectionListener(new TreeSelectionListener() {
	public void valueChanged(TreeSelectionEvent e) {
	    WoeNode node = (WoeNode) tree.getLastSelectedPathComponent();

	    if (node == null) return;

	    Object nodeInfo = node.getUserObject();
	    if (node.isLeaf()) {
		try {
		    getAppletContext().showDocument(node.getClickURL(),
						    "pageFrame");
		} catch (Exception x) { }
	    }
	}
    });

}

/** Remove all nodes except the root node. */
public void clear() {
    rootNode.removeAllChildren();
    treeModel.reload();
    nodeTable.clear();
    nodeTable.put("", rootNode);
}

public void addObject(String path, URL clickURL) {
    WoeNode childNode, parentNode;
    StringTokenizer bits = new StringTokenizer(path, ":");
    String bit, partial;
    int i, childCount;

    partial = bit = bits.nextToken();
    parentNode = rootNode;

    while (bits.hasMoreTokens()) {
	childNode = (WoeNode) nodeTable.get(partial);
	if (childNode == null) {
	    childCount = treeModel.getChildCount(parentNode);

	    for (i = 0; i < childCount; i ++) {
		childNode = (WoeNode) treeModel.getChild(parentNode, i);
		if (!treeModel.isLeaf(childNode) &&
		    bit.compareTo((String) childNode.getUserObject()) < 0) {
		    break;
		}
	    }
	    childNode = new WoeNode(bit, partial);
	    treeModel.insertNodeInto(childNode, parentNode, i);
	    nodeTable.put(partial, childNode);
	}
	bit = bits.nextToken();
	partial += ":" + bit;
	parentNode = childNode;
    }
    childCount = treeModel.getChildCount(parentNode);

    for (i = 0; i < childCount; i ++) {
	childNode = (WoeNode) treeModel.getChild(parentNode, i);
	if (!treeModel.isLeaf(childNode) ||
	    bit.compareTo((String) childNode.getUserObject()) < 0) {
	    break;
	}
    }
    childNode = new WoeNode(bit, partial, clickURL);
    treeModel.insertNodeInto(childNode, parentNode, i);
    nodeTable.put(partial, childNode);
}   

public void clearObject(String path) {
    WoeNode childNode, parentNode;
    String bit, partial;

    childNode = (WoeNode) nodeTable.get(path);
    if (childNode == null) {
	System.out.println("Could not find " + path + " node");
	return;
    }
    while (childNode != rootNode) {
	parentNode = (WoeNode) childNode.getParent();

	treeModel.removeNodeFromParent(childNode);

	nodeTable.remove(childNode.getFullPath());

	childNode = parentNode;
	/* were we the only child? if not, we're done */
	if (childNode.getChildCount() > 0) {
	    return;
	}
    }
}	

public void init() {
   hostName = getParameter("host");
   try {
      portBase = Integer.parseInt(getParameter("port"));
   } catch (Exception e) {
      portBase = 8000;
   }
}

public void start() {
    mainThread = new Thread(this);
    mainThread.start();
}

public void run() {
    Socket woeConn;
    BufferedReader woeReader;
    String pathLine;

    showStatus("Connecting to the World Tree...");

    try {
	woeConn = new Socket(hostName, portBase + 90);
    } catch (Exception e) {
	showStatus("Connection to " + hostName + ":" + portBase + " failed");
	return;
    }
    showStatus("Connected... ");
    /* we're connected; just start parsing input */

    try {
	woeReader = new BufferedReader(new InputStreamReader(
	    woeConn.getInputStream()));
    } catch (Exception e) {
	showStatus("failed to create input stream");
	return;
    }
    int loaded = 0;
    for (;;) {
	try {
	    pathLine = woeReader.readLine();
	} catch (Exception e) {
	    showStatus("connection closed");
	    return;
	}
	if (pathLine.startsWith("SET ")) {
	    int n = pathLine.indexOf(" ", 5);
	    if (n >= 0) {
		try {
		    addObject(pathLine.substring(n+1),
			      new URL("http", hostName, portBase + 80, pathLine.substring(4, n)));
		} catch (Exception e) {
		    System.out.println("Failed to make URL out of " + pathLine.substring(4, n));
		}
	    } else {
		System.out.println("Failed to find a space in " + pathLine);
	    }
	    loaded ++;
	    if ((loaded % 100) == 0) {
	       showStatus("Connected... " + loaded + " objects loaded...");
	    }
	} else if (pathLine.startsWith("CLEAR ")) {
	    clearObject(pathLine.substring(6));
	} else if (pathLine.equals("SYNC")) {
	    showStatus("Displaying!");
	    JScrollPane scrollPane = new JScrollPane(tree);
	    getContentPane().setLayout(new GridLayout(1,0));
	    getContentPane().add(scrollPane);
	    validate();
	    tree.expandPath(new TreePath(rootNode));
	}
    }
}
}

