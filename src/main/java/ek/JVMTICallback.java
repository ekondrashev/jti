package ek;

public class JVMTICallback {
    private int ccc = 0;

    public static void VMInit() {
        System.out.println("Java:\tJVMTI callback class, VMInit().");
    }

    public static void FieldModify(String name) {
        System.out.println(String.format("Java: Field modify %s", name));
    }

    public static void main(String[] args) {
        // This main is only here to give us something to run for the test

        System.out.println("Java:\tAnd Finally... Hello, I'm the Java main");
        new JVMTICallback().ccc=1;
    }

}
