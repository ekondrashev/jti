package ek;

import java.io.FileNotFoundException;

public class Main {

	private int i = 0;
	public static void main(String[] args) throws FileNotFoundException, InterruptedException {
		// TODO Auto-generated method stub
		Main m = new Main();
		m.i = 1;
		System.out.println(m.i);
		Thread.sleep(60000);
		throw new FileNotFoundException();
	}

}
