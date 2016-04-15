package ek;

import java.io.FileNotFoundException;

public class Main {

	private int i = 0;
	public static void main(String[] args) throws FileNotFoundException, InterruptedException {
		// TODO Auto-generated method stub
		Main m = new Main();
		m.i = 1;
		System.out.println(m.i);
		throw new FileNotFoundException("Resource already exists on disk: '/Tinder4MemesService/bin/.settings/org.eclipse.jdt.core.prefs'");
	}

}
