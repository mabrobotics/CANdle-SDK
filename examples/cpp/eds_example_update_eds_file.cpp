#include "OD.hpp"

using namespace mab;

edsObject MyNewObject;
edsParser MyEdsParser;

int main()
{
    // Enter the path of your eds file into the load function. once the path load you don't need to
    // reload it every time. if you want to change your eds file you call the unload method and
    // recall the load function with the of your eds file.
    // you can download an eds file at
    // https://mabrobotics.github.io/MD80-x-CANdle-Documentation/Downloads/Downloads.html#canopen-flashers
    // or use the one provide in the project at
    // CANdle-SDK/candletool/template_package/etc/candletool/config/eds/MDv1.0.0.eds
    MyEdsParser.load("/Path/to/your/eds/file.eds");

    // All the parameter of an object cf: CiA 306
    MyNewObject.index           = 0x7805;
    MyNewObject.subIndex        = 0x00;
    MyNewObject.ParameterName   = "My Custom Object";
    MyNewObject.StorageLocation = "RAM";
    MyNewObject.DataType        = 0x0007;
    MyNewObject.defaultValue    = 0x00;
    MyNewObject.accessType      = "rw";
    MyNewObject.PDOMapping      = false;
    MyNewObject.ObjectType      = 0x7;
    MyNewObject.sectionType     = OptionalObjects;

    // Add the NewObject to the eds file
    MyEdsParser.addObject(MyNewObject);

    // Display in the terminal all occurrences of the string
    MyEdsParser.find("My Custom Object");

    // Generate a html which is more convenient to read (generate in actual_Directory/eds/eds.html)
    MyEdsParser.generateHtml("/home/user/Documents/eds_parsed.html");

    // Generate a md which is more convenient to read (generate in actual_Directory/eds/eds.md)
    MyEdsParser.generateMarkdown("/home/user/Documents/eds_parsed.md");

    // Generate a cpp file which can be for verification in your software
    MyEdsParser.generateCpp("/home/user/Documents/");

    return EXIT_SUCCESS;
}