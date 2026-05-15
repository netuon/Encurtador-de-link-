#include <iostream>
#include "crow.h" // Biblioteca Crow para criar o servidor web
#include <pqxx/pqxx> // Biblioteca pqxx para conectar ao banco de dados PostgreSQL
#include <string>
#include <random>
#include <algorithm>

using namespace std;

string gerarCodigo(size_t tamanho){
        const string caracteres =
            "0123456789"
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> dis(0, caracteres.size() -1);

            string s;

            for(size_t i = 0; i < tamanho; i++){
                s += caracteres[dis(gen)];
            }
            return s;
    }



int main(){
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

    CROW_ROUTE(app, "/receber").methods(crow::HTTPMethod::POST, crow::HTTPMethod::OPTIONS)
    ([](const crow::request& req){
        if (req.method == crow::HTTPMethod::OPTIONS){
            crow::response res(200);
            cout << "Options recebido" << endl;
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type");

            return res;
        } else {
            pqxx::connection c("dbname=encurtador user=postgres password=1234 host=127.0.0.1");

            try{
                if(c.is_open()){
                    cout << "Conectado com sucesso!" << endl;
                } 
            } catch (const pqxx::sql_error &e) {
                cerr << "erro:" << e.what() << endl;//imprime qualquer erro que ocorrer durante a conexão ou consulta
            }

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


            string codigo = gerarCodigo(6);
            pqxx::work tx{c};

            //monta o json de resposta
            crow::response res(200);
            res.add_header("Access-Control-Allow-Origin", "*");
            crow::json::wvalue response;
            response["url"] = "http://localhost:18080/" + codigo;
            res.body = response.dump();
            res.add_header("Content-Type", "application/json");//cabeçalho

            tx.exec("INSERT INTO urls(codigo, url_original) VALUES('" + codigo + "', '"+ url +"')");
            tx.commit();

            return res;
        }
        
    });

    CROW_ROUTE(app, "/<string>")
    ([](const string& codigo){
        pqxx::connection c("dbname=encurtador user=postgres password=1234 host=127.0.0.1");

        pqxx::work tx{c};
        pqxx::result r = tx.exec("SELECT url_original FROM urls WHERE codigo = '" + codigo + "'");
        
        if(r.empty()){
            return crow::response(404);
        }

        tx.commit();
        crow::response res(301);//301 serve para rederecionar o user
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Location", r[0][0].as<string>());

        return res;
    });
    

    app.port(18080).multithreaded().run();//define a porta do servidor e inicia o servidor web

}
