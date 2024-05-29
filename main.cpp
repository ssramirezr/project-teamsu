#include <iostream>
#include <vector>
#include <regex>
#include <iostream>
#include <cctype>
#include <string>
#include <unordered_set>
#include <set>
#include <algorithm>


int numTotGrams;
int numProds;
int numStrings;
using namespace std;

// Utilidad, eliminar duplicados en un vector..
void eliminarDuplicados(vector<string>& vec) {
    unordered_set<string> elementosUnicos;
    auto it = vec.begin();
    while (it != vec.end()) {
        if (elementosUnicos.find(*it) != elementosUnicos.end()) {
            it = vec.erase(it);
        } else {
            elementosUnicos.insert(*it);
            ++it;
        }
    }
}

vector<string> split(const string& str) {
    vector<string> result;
    istringstream iss(str);
    string word;
    while (iss >> word) {
        result.push_back(word);
    }
    return result;
}

vector<string> splitEp(const string& str) {
    vector<string> result;
    istringstream iss(str);
    string word;
    while (iss >> word) {
        result.push_back("%"+word);
    }
    return result;
}

// utilidad para propagate update.
set<string> vectorToSet(const vector<string>& vec) {
    return set<string>(vec.begin(), vec.end());
}

// Función para propagar actualizaciones
void propagateUpdate(const string& vectorName, map<string, vector<string>>& vectors, map<string, vector<string>>& dependencies) {
    if (dependencies.find(vectorName) != dependencies.end()) {
        for (const string& dependentVector : dependencies[vectorName]) {// se convierten vectores a sets para facilitar la unión
            set<string> currentSet = vectorToSet(vectors[vectorName]);
            set<string> dependentSet = vectorToSet(vectors[dependentVector]);// unimos los elementos del vector actual al vector dependiente

            dependentSet.insert(currentSet.begin(), currentSet.end());// se convierte  el set de vuelta a un vector
            vectors[dependentVector] = vector<string>(dependentSet.begin(), dependentSet.end());// Propagar la actualización al siguiente nivel

            propagateUpdate(dependentVector, vectors, dependencies);
        }
    }
}

bool IsInVector(const std::vector<std::string>& vec, const std::string& elemento) {
    return find(vec.begin(), vec.end(), elemento) != vec.end(); // / find retorna un iterador al elemento si lo encuentra, o vec.end() si no lo encuentra.
}


// Utilidad, traer los first de un símbolo terminal.
vector<string> searchFirstOf(vector<vector<string>> firsts, string firstOf){
    for(int i=0; i<firsts.size(); i++ ){
        if(firsts[i][0]==firstOf){
            return firsts[i];
        }
    }
}


// First recursivo.
vector<string> computeFirstIn(vector<vector<string>> grammar, int numProdsOfGrammar, string uppFirstOf){
    vector<string> firstVector;
    for(int i=0; i<numProdsOfGrammar; i++){
        if(grammar[i][0]==uppFirstOf){ // Iteramos sólo a través de las producciones de uppFirstOf (terminal)
            for(int j=1; j<grammar[i].size() ; j++){//
                string firstLetter = grammar[i][j]; // Cadena de la producción para luego llamar su primera letra
                if(isupper(firstLetter[0]) != 1){ // En caso de que el primer símbolo sea terminal, se añade directamente
                    string toAddChar = string(1, firstLetter[0]);
                    firstVector.push_back(toAddChar);
                } else if(isupper(firstLetter[0]) == 1 && string(1, firstLetter[0]) != uppFirstOf) { // si no es terminal y no A->A
                    vector<string> listOfIn = computeFirstIn(grammar, numProdsOfGrammar, string(1, firstLetter[0])); // Recursividad
                    for (string element : listOfIn){// Se añade los First que se trajeron de la recursividad menos e.
                        if(element!="e"){ // First(x)-e
                            firstVector.push_back(element);
                        }
                    }
                }
            }
        }
    }
    return firstVector;
}

vector<vector<string>> computeFirst(vector<vector<string>> grammar, int numProdsOfGrammar){
    vector<vector<string>> firstOfEvery;
    //Bloque para calcular first en primera instancia.
    for(int i=0; i<numProdsOfGrammar; i++){
        vector<string> firstOf;
        firstOf.clear();
        firstOf.push_back(grammar[i][0]); // inicializamos cada vector de los first con su terminal correspondiente.
        vector<string> listIn = computeFirstIn(grammar, numProdsOfGrammar, (1, firstOf[0]) ); // llamamos el algoritmo recursivo.

        for (string element : listIn){ // Almacenamos los elementos first calculados en la función recursiva.
            firstOf.push_back(element);
        }

        eliminarDuplicados(firstOf); // Muchos elementos quedan duplicados.
        firstOfEvery.push_back(firstOf); // Añadimo
    }

    //Bloque para calcular first en segunda instancia: casos que involucran epsilon como:  S->Ab A->e
    for(int i=0; i<numProdsOfGrammar; i++){
        for(int j=1; j<grammar[i].size(); j++ ){
            string prodToEvaluate = grammar[i][j];
            if(isupper(prodToEvaluate[0]) && prodToEvaluate.size()>=2 ){ // Vemos si la producción tiene 2 o más caracteres e inicia con un No terminal
                vector<string> listFirstOf = searchFirstOf(firstOfEvery, string(1,prodToEvaluate[0])); // Se sacan las producciones del Noterminal que se encuentra al principio del string para ver si tiene una prod EPsilonm
                if(IsInVector(listFirstOf, "e")){ // la producción epsilon está en los first del terminal?
                    if(isupper(prodToEvaluate[1])){ // first de grammar[i}[j[1] se mandan a el first de grammar[i][0]
                        vector<string> listFirstOfNext = searchFirstOf(firstOfEvery, string(1,prodToEvaluate[1]));
                        for(int x=1; x<listFirstOfNext.size();x++){
                            firstOfEvery[i].push_back(listFirstOfNext[x]);
                            eliminarDuplicados(firstOfEvery[i]);
                        }
                    } else {
                        firstOfEvery[i].push_back(string(1,prodToEvaluate[1]));
                        eliminarDuplicados(firstOfEvery[i]);
                    }
                }
            } if(isupper(prodToEvaluate[0]) && prodToEvaluate.size()==1 ){ // Caso S->A, A->e
                vector<string> listFirstOf = searchFirstOf(firstOfEvery, string(1,prodToEvaluate[0])); // Se sacan las producciones del terminal que se encuentra al principio del string para ver si tiene una prod EPsilonm
                if(IsInVector(listFirstOf, "e")){ // Si A->e entonces first(A) contiene e.
                    firstOfEvery[i].push_back("e");
                    eliminarDuplicados(firstOfEvery[i]);
                }
            } if(isupper(prodToEvaluate[0]) && prodToEvaluate.size()>1){ // En caso de tener cadenas S->ABE y estos terminales deriven en epsilon.
                for(int y=0; y<prodToEvaluate.size(); y++){
                    if(isupper(prodToEvaluate[y])){
                        vector<string> listFirstOf = searchFirstOf(firstOfEvery, string(1,prodToEvaluate[y]));
                        if(IsInVector(listFirstOf, "e")){
                            if(y==prodToEvaluate.size()-1){ // Si se llega al último terminal de la producción y este también deriva en e, se añade e.
                                firstOfEvery[i].push_back("e");
                                eliminarDuplicados(firstOfEvery[i]);
                            }
                        } else {
                            break;
                        }
                    }
                }
            }
        }
    }

        return firstOfEvery;
}



map<string, vector<string>> computeFollow(vector<vector<string>> grammar, vector<vector<string>> grammarEp, vector<vector<string>> firsts, int numProdsOfGrammar){
    map< string, vector<string> > follows;//
    for(int i=0; i<numProdsOfGrammar; i++){ // Se agrega $ a todos los follows.
        follows[grammar[i][0]].push_back("$");
    }

    for(int i=0; i<grammar.size(); i++){
        for(int j=1; j<grammar[i].size(); j++){
            if(grammarEp[i][j].size()>2){
                for(int x=0; x<grammarEp[i][j].size()-2; x++){ // Vamos moviendo alpha, B y Beta a lo largo de la producción.
                    string actualProd = grammarEp[i][j];
                    string alpha = string(1, actualProd[x]) ;
                    string B = string(1, actualProd[x+1]);
                    string beta = string(1, actualProd[x+2]);

                    char BChar = actualProd[x+1];
                    char betaChar = actualProd[x+2];

                    //Si hay producciones A->αBβ, se añade First(β)-e a Follow(B)
                    if(isupper(BChar) == 1){ // B es terminal
                        if(isupper(betaChar) ==1 ){
                            vector<string> tempList = searchFirstOf(firsts, beta);
                            // Añadimos los first de β, en caso de que solo haya un first se agrega ese, si hay varios simbolos se agregan a partir del indice 1.
                            if(tempList.size()==2){
                                follows[B].push_back(tempList[1]);
                            } else if(tempList.size()>2) {
                                for(int y=1; y< searchFirstOf(firsts, beta).size(); y++){
                                    if(tempList[y]!="e"){ // !No agregar e a los follows.
                                        follows[B].push_back(tempList[y]);
                                    }
                                }
                            }

                            // en caso de que first(β) tenga epsilon, se miran los siguientes símbolos.
                            if(IsInVector(tempList, "e")){
                                string restante = grammarEp[i][j].substr(x+2);
                                if(restante.size() > 1 ){
                                    for(int z=1; z<restante.size(); z++){// se itera por la cadena restante.
                                        if(isupper(restante[z])){// agregamos los first de la string restante en consecuencia.
                                            vector<string> tempList2 = searchFirstOf(firsts, string(1,restante[z]));

                                            if(tempList2.size()==2){
                                                follows[B].push_back(tempList2[1]);
                                            } else if(tempList2.size()>2) {
                                                for(int y=1; y< searchFirstOf(firsts, beta).size(); y++){
                                                    if(tempList2[y]!="e"){
                                                        follows[B].push_back(tempList2[y]);
                                                    }

                                                }
                                            }

                                            if(!IsInVector(tempList, "e")){// revisar si tiene epsilon en sus cadenas. si no, BREAK
                                                break;
                                            }
                                        } else {
                                            follows[B].push_back(string(1,restante[z]));
                                        }
                                    }
                                }
                            }

                        } else { // beta es no terminal, simplemente se añade.
                            follows[B].push_back(beta);
                        }

                    }

                }
            }
        }
    }

    // If there is a production A → αB, or a production A → αBβ with ε ∈ First(β), then add
    // Follow(A) to Follow(B).
    map<string, vector<string>> dependencies;
    for(int i=0; i<grammar.size(); i++){
        for(int j=1; j<grammar[i].size(); j++){
            int prodSize = grammar[i][j].size();
            string actualProd = grammar[i][j];
            char lastCharProd = actualProd[prodSize - 1];
            if(isupper(lastCharProd)){ // Siempre que haya un B o β-terminal para verificar si tiene producción epsilon
                if(grammar[i][0]!=string(1,lastCharProd)) { // Se evita Follow(A) to Follow(A).
                    dependencies[grammar[i][0]].push_back(string(1,lastCharProd)); // se añaden las dependencias.
                }

                // Se verifica si e pertenece a first(β)
                vector<string> firstOfC = searchFirstOf(firsts, string(1,lastCharProd));
                if(IsInVector(firstOfC, "e") && prodSize>=2){
                    for(int x=2; x <= prodSize; x++){ // bucle hasta que deje de ser epsilon.
                        if(isupper(actualProd[prodSize - x])){// Vamos al terminal de antes

                            if(grammar[i][0]!=string(1,actualProd[prodSize - x])) {dependencies[grammar[i][0]].push_back(string(1,actualProd[prodSize - x]));}
                             // Se añade dependencia del anterior terminal.

                            vector<string> firstOfBack = searchFirstOf(firsts, string(1,actualProd[prodSize - x]));
                            if(!IsInVector(firstOfBack, "e")){
                                break;
                            }
                        }
                    }
                }
            }

        }
    }

    for(int i=0; i<grammar.size(); i++){
        propagateUpdate(grammar[i][0], follows, dependencies); // Se propagan todas las dependencias y se actualizan los conjuntos.
    }

    cout << endl;

    //Impresión de follows.
    for (const auto& par : follows) {
        cout << "Follow(" << par.first << ") = {";
        bool first = true;
        for (const auto& valor : par.second) {
            if (!first) {
                cout << ",";
            }
            cout << valor;
            first = false;
        }
        cout << "}" << endl;
    }


    return follows;
}


int main() {
    // Se hace la lectura inicial del total de gramáticas
    scanf("%d", &numTotGrams);
    for(int i = 0; i<numTotGrams; i++){
        scanf("%d", &numProds); // Se hace la lectura inicial para cada gramática
        scanf("%d", &numStrings);
        vector<string> lines;
        vector<vector<string>> grammar;
        vector<vector<string>> grammarEp; // Gramática a la que se le añade un símbolo que representa epsilon al principio de cada producción


        for(int j = 0; j<numProds; j++){
            string line;
            getline(cin, line);
            vector<string> gramProd = split(line);
            vector<string> gramProdEp = splitEp(line); // Se guardan las gramáticas en vectores y posteriormente en matrices.
            grammar.push_back(gramProd);
            grammarEp.push_back(gramProdEp);
        }

        vector<vector<string>> firsts = computeFirst(grammar, numProds); // Calculamos la matriz de firsts de la gramática y la guardamos.
        for (int i=0; i<firsts.size(); i++) { //impresión de first.
            for (int j=0; j<firsts[i].size(); j++) {
                if(j==0) {
                    cout<<"First("<<firsts[i][j]<<") = {";
                } else if(j==firsts[i].size()-1) {
                    cout << firsts[i][j];
                } else {
                    cout << firsts[i][j] << ",";
                }
            }
            cout << "}";
            if(i!=firsts.size()-1){
                cout << endl;
            }

        }

        map< string, vector<string> > diccionario = computeFollow(grammar, grammarEp, firsts, numProds);
        
    }
}