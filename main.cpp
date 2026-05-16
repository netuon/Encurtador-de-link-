#include <iostream>
#include "crow.h" // Biblioteca Crow para criar o servidor web
#include <pqxx/pqxx> // Biblioteca pqxx para conectar ao banco de dados PostgreSQL
#include <string>
#include <random>
#include <algorithm>
#include <cstdlib>

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
    ([](const crow::request& req) {
        // 1. Criamos a resposta ÚNICA que vai ser usada em toda a rota
        crow::response res;
        
        // 2. Adiciona os headers de CORS logo no início
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        res.add_header("Content-Type", "application/json");

        // 3. Se for OPTIONS (checagem do navegador), responde 200 OK imediatamente
        if (req.method == crow::HTTPMethod::OPTIONS) {
            res.code = 200;
            return res;
        }

        // 4. Se for POST, segue o processo:
        const char* db_url = std::getenv("DATABASE_URL");
        pqxx::connection c(db_url);

        try {
            if(c.is_open()){
                cout << "Conectado com sucesso ao Neon!" << endl;
            } 
        } catch (const pqxx::sql_error &e) {
            cerr << "Erro no banco: " << e.what() << endl;
            res.code = 500;
            res.body = "{\"error\": \"Erro no banco de dados\"}";
            return res;
        }

        auto body = crow::json::load(req.body);
        if (!body){
            res.code = 400;
            res.body = "{\"error\": \"JSON invalido\"}";
            return res;
        }

        string url_user = body["url"].s();
        cout << "URL recebida: " << url_user << endl;

        string codigo = gerarCodigo(6);
        pqxx::work tx{c};

        // Salva no banco de dados Neon
        tx.exec("INSERT INTO urls(codigo, url_original) VALUES('" + codigo + "', '" + url_user + "')");
        tx.commit();

        // Monta o JSON de sucesso (AQUI FOI CORRIGIDO: não recriamos 'res', apenas alteramos ela)
        crow::json::wvalue response;
        response["url"] = "https://encurtador-de-link-zfeq.onrender.com/" + codigo;
        
        res.code = 200;
        res.body = response.dump();

        return res; 
    });

    CROW_ROUTE(app, "/<string>")
    ([](const string& codigo){
        const char* db_url = std::getenv("DATABASE_URL");
        pqxx::connection c(db_url);

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
    

    app.port(10000).multithreaded().run();//define a porta do servidor e inicia o servidor web

}
