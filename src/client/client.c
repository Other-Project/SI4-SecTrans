#include <stdio.h>
#include <string.h>
#include "client.h"
#include "common.h"

int main(int argc, char **argv)
{
    /*if (argc < 2)
    {
        printf("Missing argument\n");
        return 1;
    }*/

    printf("Starting client\n");
    char msg[] = "Lorem ipsum odor amet, consectetuer adipiscing elit. Lectus vel efficitur aptent purus augue sit platea. Cubilia nostra natoque convallis habitant sem quisque aliquet. Metus habitant dis condimentum vehicula natoque aptent consectetur. Praesent scelerisque penatibus ultricies luctus velit ultrices fusce odio. Urna in curabitur odio felis condimentum commodo enim risus. Volutpat felis dui quam in ultricies vivamus tincidunt pellentesque. Quis sit vehicula volutpat parturient nec. Aenean congue per mus quam congue tempus dictum. Pellentesque penatibus posuere luctus tincidunt aliquam metus. Ornare vivamus ultrices, penatibus vestibulum suscipit etiam magna. Convallis dis torquent felis etiam potenti porta dictum. Iaculis porta ut euismod interdum nibh elit commodo quam. Egestas eros curabitur eu blandit nec. Curae duis per lacinia lacus convallis arcu class. Per aliquet himenaeos tempus tortor magna ut sem per senectus. Consequat nascetur mollis penatibus scelerisque dis primis laoreet porta. Velit tempor sagittis hendrerit venenatis nibh a. Hac nunc duis magnis hendrerit quisque. Praesent lorem primis porta accumsan lobortis sagittis curabitur iaculis. Eu orci accumsan litora, vehicula rhoncus consequat. Massa urna fermentum erat curabitur egestas hendrerit sit magna. Dolor penatibus bibendum id sagittis dictumst lacus. Quis curabitur habitant cras turpis nullam; ut velit ac. Inceptos eros netus dui imperdiet elit ornare vel. Phasellus curabitur feugiat aptent bibendum senectus convallis. Integer platea taciti vel tellus, dapibus cras porttitor. Feugiat posuere felis scelerisque integer mattis aliquam conubia pharetra. Efficitur hendrerit phasellus parturient luctus scelerisque habitant dignissim. Rutrum accumsan mi mollis consequat proin posuere sollicitudin. Facilisis scelerisque ligula a posuere sollicitudin sociosqu suspendisse. Praesent ultrices eleifend phasellus urna egestas quis ut elementum. Vulputate viverra dapibus; blandit odio eros viverra tortor fermentum. Rhoncus purus lorem pharetra suscipit nisl tellus. Eu himenaeos arcu mauris egestas nisi. Duis semper rhoncus lacus conubia sit. Torquent lacinia parturient lectus, pharetra eu sodales. Etiam id cursus per senectus habitasse primis a diam. At ultrices donec tristique maximus; natoque nisl condimentum. Iaculis rutrum eleifend vulputate eros fermentum blandit iaculis. Cubilia at rhoncus neque rhoncus mauris parturient nam quisque. Senectus vitae ante; ac interdum ad fusce. Vestibulum malesuada dolor velit orci ullamcorper hendrerit, venenatis vehicula. Per class laoreet efficitur fermentum eleifend orci magnis nibh.";
    if(safe_send_message(msg, SERVER_PORT)) {
        printf("Error sending message\n");
        return 1;
    }
    return 0;
}
