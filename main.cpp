#include <iostream>
#include "crow.h" // Biblioteca Crow para criar o servidor web
#include <pqxx/pqxx> // Biblioteca pqxx para conectar ao banco de dados PostgreSQL
#include <string>

using namespace std;


int main(){
    try{
        pqxx::connection c("dbname=encurtador user=postgres password=1234 host=127.0.0.1");//conexão com o banco de dados PostgreSQL

        if(c.is_open()){
            cout << "Conectado com sucesso!" << endl;
        } 
    } catch (const pqxx::sql_error &e) {
        cerr << "erro:" << e.what() << endl;//imprime qualquer erro que ocorrer durante a conexão ou consulta
    }

    crow::SimpleApp app;//criação do servidor web usando a biblioteca Crow


    CROW_ROUTE(app, "/pagina")([](){
        char name[100];//variavel para armazenar o nome do servidor
        gethostname(name, 100);//pega o nome do servidor
        crow::mustache::context x;
        crow::mustache::set_base("templates");//conjunto de dados a ser enviado ao template
        x["servername"] = name;//no html vai existir um variavel username, e o valor dela vai ser o nome do servidor

        auto page = crow::mustache::load("pagina.html");
        return page.render(x);
    });


    CROW_ROUTE(app, "/receber").methods(crow::HTTPMethod::POST)
    ([](const crow::request& req){

        //faz a transformação do json recebido
        auto body = crow::json::load(req.body);
        if (!body){
            return crow::response(400);
        }

        //captura a url do json recebido
        string url = body["url"].s();

        //faz o que precisar com a URL
        cout << "URL recebida: " << url << endl;
        string processado = "C++ recebeu: " + url;

        //monta o json de resposta
        crow::json::wvalue response;
        response["url"] = "requisição concluida com sucesso!" + url;


        return crow::response(response);
         });
         
    
    

    app.port(18080).multithreaded().run();//define a porta do servidor e inicia o servidor web

}
