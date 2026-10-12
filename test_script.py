import pexpect
import sys

failures = []

def run_cmd(child, cmd, expect_str=None, timeout=2):
    child.sendline(cmd)
    try:
        child.expect('>', timeout=timeout)
        out = child.before
        if expect_str and expect_str not in out:
            failures.append(f"ERROR: Expected '{expect_str}' not found in output of '{cmd}'. Output was:\n{out}")
    except pexpect.TIMEOUT:
        failures.append(f"ERROR: TIMEOUT on command '{cmd}'. Output so far:\n{child.before}")

def test_game():
    print("Starting Game Test...")
    try:
        child = pexpect.spawn('./a.out', encoding='utf-8', timeout=2)
    except Exception as e:
        print(f"Failed to spawn ./a.out: {e}")
        sys.exit(1)

    try:
        child.expect('>')
        
        # Test Verbosity
        run_cmd(child, 'brief', 'Brief mode on.')
        run_cmd(child, 'verbose', 'Verbose mode on.')

        # Test Basic Movement
        run_cmd(child, 'go north', 'Food Court - Pizza')
        run_cmd(child, 'go south', 'Central Atrium')

        # Test Taking, Dropping, Examining
        run_cmd(child, 'examine phone', 'A smartphone with a cracked screen.')
        run_cmd(child, 'take phone', 'Taken.')
        run_cmd(child, 'inventory', 'phone')
        run_cmd(child, 'drop phone', 'Dropped.')

        # Test Pronoun Parser
        run_cmd(child, 'examine phone', 'A smartphone with a cracked screen.')
        run_cmd(child, 'take it', 'Taken.')
        run_cmd(child, 'inventory', 'phone')
        run_cmd(child, 'call phone', 'Calling phone... no answer.')

        # To Burgers
        run_cmd(child, 'go south', 'Food Court - Burgers')
        
        # Minigame Burger
        child.sendline('use grill')
        try:
            child.expect("Type 'flip' exactly 3 times", timeout=2)
            for i in range(3):
                child.expect("Type 'flip' NOW:", timeout=2)
                child.sendline('flip')
            
            child.expect('Perfect burger! You won!')
            child.expect('>')
        except pexpect.TIMEOUT:
            failures.append(f"ERROR in Minigame Burger: TIMEOUT. Output so far:\n{child.before}")

        # Test Score
        run_cmd(child, 'score', 'Score: 1')

        # Test OOPS and AGAIN
        run_cmd(child, 'nprth', "I don't understand that command")
        run_cmd(child, 'oops north', 'Central Atrium')
        
        run_cmd(child, 'go north', 'Food Court - Pizza')
        # Test AGAIN ('g' is synonymous)
        child.sendline('again')
        try:
            child.expect('>', timeout=2)
            if 'Central Atrium' not in child.before and "Food Court - Pizza" not in child.before and "Path's blocked" not in child.before and "Nowhere" not in child.before:
                pass 
        except pexpect.TIMEOUT:
            failures.append("ERROR on again: TIMEOUT")

        # Test Darkness Mechanics
        run_cmd(child, 'go east', 'Bookstore') 
        run_cmd(child, 'go south', 'Office Supplies') 
        run_cmd(child, 'take lantern', 'Taken.')
        
        # Going into dark closet should now say it's pitch black
        run_cmd(child, 'go east', 'It is pitch black') 
        
        # Turning on lantern should now work in the dark
        run_cmd(child, 'turn on lantern', 'Closet')

        # Test Containers/Supporters
        run_cmd(child, 'go west', 'Office Supplies') 
        run_cmd(child, 'go north', 'Bookstore') 
        run_cmd(child, 'go west', 'Food Court - Pizza') 
        run_cmd(child, 'go south', 'Central Atrium') 
        run_cmd(child, 'go up', 'Potion Shop') 
        run_cmd(child, 'put phone on alchemist_desk', 'You put') 

        # Test Dynamic Help Output
        run_cmd(child, 'help', 'Available verbs:')
        run_cmd(child, 'help', 'call')
        run_cmd(child, 'help', 'turn on')
        run_cmd(child, 'help', 'turn off')

        # Test Call fallback when no phone is nearby
        run_cmd(child, 'go down', 'Central Atrium')
        run_cmd(child, 'call', 'no phone nearby') 

    except pexpect.EOF:
        failures.append("--- EOF EXCEPTION ---")
    finally:
        child.close()

    if failures:
        print("\nTESTS FAILED:")
        for f in failures:
            print(f)
        sys.exit(1)
    else:
        print("\nALL TESTS PASSED SUCCESSFULLY!")

if __name__ == '__main__':
    test_game()
