# Function to generate a YANG model dynamically
def generate_yang_module(module_name, namespace, prefix, container_name, leaf_name, leaf_type):
    yang_model = """
    module ietf-mud{
        container mud{
            leaf mud-version{
                type uint8;
            }
            leaf mud-url{
                type inet:uri;
            }
            leaf mud-signature{
                type inet:uri;
            }
            leaf last-update{
                type yang:date-and-time;
            }
            leaf cache-validity{
                type uint8;
            }
            leaf is-supported{
                type boolean;
            }
            leaf-list systeminfo{
                type string;
            }
            leaf mfg-name{
                type string;
            }
            leaf model-name{
                type string;
            }
            leaf firmware-rev{
                type string;
            }
            leaf software-rev{
                type string;
            }
            leaf documentation{
                type inet:uri;
            }
            leaf extensions*{
                type string;
            }
            container from-device-policy{
                leaf-list acls*{
                    leaf name{
                        type string;
                    }
                }
            }
            container to-device-policy{
                leaf-list acl-name{
                    type string;
                }
            }
        }
    }
    """
    return yang_model

# Generate a YANG model
module_name = "aaadynamic-module"
namespace = "http://example.com/ns/dynamic-module"
prefix = "dyn"
container_name = "dynamic-container"
leaf_name = "dynamic-leaf"
leaf_type = "string"

yang_model = generate_yang_module(module_name, namespace, prefix, container_name, leaf_name, leaf_type)

# Write the YANG model to a file
file_name = f"{module_name}.yang"
with open(file_name, "w") as yang_file:
    yang_file.write(yang_model)

print(f"YANG model written to {file_name}")
