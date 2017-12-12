package apriorit.windowscredential;

import java.util.regex.Pattern;

/**
 * Created by Danil on 10.12.2017.
 */

//volidate IP
public class IPAddressValidator {

    private static final String IP_ADDRESS_PATTERN
            = "^([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
            + "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
            + "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\."
            + "([01]?\\d\\d?|2[0-4]\\d|25[0-5])$";

    private final Pattern pattern;

    public IPAddressValidator() {
        pattern = Pattern.compile(IP_ADDRESS_PATTERN);
    }

    public boolean validate(String ipAddress) {
        return pattern.matcher(ipAddress).matches();
    }
}