def duo():
    try:
        print("duo")
        raise NameError("Error in duo")
    except ValueError as e:
        print("errorrrrrorror")
        raise

def uno():
    n = 0
    for i in range(5):
        try:
            n+=1
            print(n)
            if n == 3:
                raise ValueError("Error in uno")
        except ValueError as e:
            print(f"Error in uno: {e}")
            
        finally:
            print("finally in uno")

duo()