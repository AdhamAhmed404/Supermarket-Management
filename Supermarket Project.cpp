#include <iostream>

using namespace std;

string lower(string str) {
    for (char &c : str) {
        if (c >= 'A' && c <= 'Z') {
            c += 32;
        }
    }
    return str;
}

class Product {
private:
    int id;
    string name;
    string category;
    double price;
    int quantity;
    int soldCount;
public:
    Product(int pid, string pname, string pcategory, double pprice, int pqty) : id(pid), name(pname), category(pcategory), price(pprice), quantity(pqty), soldCount(0) {}

    int getId() {return id;}
    string getName() {return name;}
    string getCategory() {return category;}
    double getPrice() {return price;}
    int getQuantity() {return quantity;}
    int getSoldCount() { return soldCount;}

    void setId(int i) {id = i;}
    void setName(string n) {name = n;}
    void setCategory(string c) {category = c;}
    void setPrice(double p) {price = p;}
    void setQuantity(int q) {quantity = q;}
    void addSold(int qty) { soldCount += qty;}

    void displayProduct() {
        cout << "===============================" << endl;
        cout << "Product ID   : " << id << endl;
        cout << "Name         : " << name << endl;
        cout << "Category     : " << category << endl;
        cout << "Price        : $" << price << endl;
        cout << "Available Qty: " << quantity << endl;
        cout << "===============================" << endl;
    }
};

class Trie {
private:
    struct TrieNode {
        TrieNode* children[26];
        bool isEnd;

        TrieNode() {
            isEnd = false;
            for(int i = 0; i < 26; i++)
                children[i] = nullptr;
        }
    };

    TrieNode* root;

    void autocompleteHelper(TrieNode* node, string prefix) {
        if(node->isEnd) {
            cout << prefix << endl;
        }

        for(int i = 0; i < 26; i++) {
            if(node->children[i]) {
                char nextChar = 'a' + i;
                autocompleteHelper(node->children[i], prefix + nextChar);
            }
        }
    }

    void deleteTrie(TrieNode* node) {
        if (!node) return;
        for(int i = 0; i < 26; i++) {
            if(node->children[i]) {
                deleteTrie(node->children[i]);
            }
        }
        delete node;
    }

public:
    Trie() {
        root = new TrieNode();
    }

    void insert(string word) {
        word = lower(word);
        TrieNode* curr = root;
        for(char c : word) {
            int index = c - 'a';
            if(index < 0 || index >= 26) continue;

            if(!curr->children[index])
                curr->children[index] = new TrieNode();

            curr = curr->children[index];
        }
        curr->isEnd = true;
    }

    void autocomplete(string prefix) {
        prefix = lower(prefix);
        TrieNode* curr = root;

        for(char c : prefix) {
            int index = c - 'a';
            if(index < 0 || index >= 26 || !curr->children[index]) {
                cout << "No suggestions found.\n";
                return;
            }
            curr = curr->children[index];
        }

        cout << "Suggestions:\n";
        autocompleteHelper(curr, prefix);
    }

    ~Trie() {
        deleteTrie(root);
    }
};

class Cart {
private:
    struct CartNode {
        Product* product;
        int quantity;
        CartNode* next;

        CartNode(Product* prod, int qty, CartNode* n = nullptr) : product(prod), quantity(qty), next(n) {}
    };
    struct UndoNode {
        string action;
        Product* product;
        int oldQty;
        int newQty;
        UndoNode* next;

        UndoNode(string a, Product* p, int o, int n) : action(a), product(p), oldQty(o), newQty(n), next(nullptr) {}
    };

    UndoNode* undoTop;
    CartNode* head;

public:
    Cart() : head(nullptr), undoTop(nullptr) {}

    void addItem(Product* prod, int qty) {
        CartNode* ptr = head;
        while(ptr) {
            if(ptr->product->getId() == prod->getId()) {
                int old = ptr->quantity;
                ptr->quantity += qty;
                pushUndo("update", prod, old, ptr->quantity);
                return;
            }
            ptr = ptr->next;
        }
        head = new CartNode(prod, qty, head);
        pushUndo("add", prod, 0, qty);
    }

    void removeItem(int id) {
        CartNode* curr = head;
        CartNode* prev = nullptr;
        while(curr) {
            if(curr->product->getId() == id) {
                pushUndo("remove", curr->product, curr->quantity, 0);
                if(prev) prev->next = curr->next;
                else head = curr->next;
                delete curr;
                return;
            }
            prev = curr;
            curr = curr->next;
        }
    }

    void updateItemQuantity(int id, int qty) {
        CartNode* ptr = head;
        while(ptr) {
            if(ptr->product->getId() == id) {
                pushUndo("update", ptr->product, ptr->quantity, qty);
                ptr->quantity = qty;
                return;
            }
            ptr = ptr->next;
        }
    }

    void viewCart() {
        if (!head) {
            cout << "Cart is empty." << endl;
            return;
        }
        CartNode* ptr = head;
        cout << "===== CART ITEMS =====" << endl;
        while (ptr) {
            cout << "ID: " << ptr->product->getId() << endl;
            cout << "Name: " << ptr->product->getName() << endl;
            cout << "Quantity: " << ptr->quantity << endl;
            cout << "Price: " << ptr->product->getPrice() << endl;
            cout << "Subtotal: " << ptr->quantity * ptr->product->getPrice() << endl;
            cout << "----------------------" << endl;
            ptr = ptr->next;
        }
    }

    double calculateTotal() {
        double total = 0.0;
        CartNode* ptr = head;
        while(ptr) {
            total += ptr -> product -> getPrice() * ptr -> quantity;
            ptr = ptr -> next;
        }
        return total;
    }

    void applyOrderToInventory() {
        CartNode* ptr = head;
        while(ptr) {
            Product* prod = ptr->product;
            int available = prod->getQuantity();
            int ordered = ptr->quantity;

            if(ordered <= available) {
                prod->setQuantity(available - ordered);
                prod->addSold(ordered);
            } else {
                cout << "Not enough stock for " << prod->getName() << endl;
            }
            ptr = ptr->next;
        }
    }

    void pushUndo(string action, Product* prod, int oldQty, int newQty) {
        UndoNode* node = new UndoNode(action, prod, oldQty, newQty);
        node->next = undoTop;
        undoTop = node;
    }

    UndoNode* popUndo() {
        if(!undoTop) return nullptr;
        UndoNode* temp = undoTop;
        undoTop = undoTop->next;
        return temp;
    }

    void undoLastAction() {
        UndoNode* last = popUndo();
        if(!last) {
            cout << "Nothing to undo.\n";
            return;
        }
        if(last->action == "add") {
            removeItem(last->product->getId());
            delete popUndo();
            cout << "Undo: Item removed.\n";
        }
        else if(last->action == "remove") {
            addItem(last->product, last->oldQty);
            delete popUndo();
            cout << "Undo: Item restored.\n";
        }
        else if(last->action == "update") {
            updateItemQuantity(last->product->getId(), last->oldQty);
            delete popUndo();
            cout << "Undo: Quantity restored.\n";
        }

        delete last;
    }

    void clear() {
        while(head) {
            CartNode* temp = head;
            head = head->next;
            delete temp;
        }
    }

    ~Cart() {
        while(head) {
            CartNode* temp = head;
            head = head->next;
            delete temp;
        }
        while(undoTop) {
            UndoNode* temp = undoTop;
            undoTop = undoTop->next;
            delete temp;
        }
    }
};

class Inventory {
private:
    struct InventoryNode {
        Product* product;
        InventoryNode* next;

        InventoryNode(Product* prod, InventoryNode* n = nullptr) : product(prod), next(n) {}
    };

    InventoryNode* head;
    Trie* productTrie;
public:
    Inventory() : head(nullptr) {
        productTrie = new Trie();
    }

    void addNewProduct(int id, string name, string cat, double price, int qty) {
        name = lower(name);
        cat = lower(cat);
        InventoryNode* ptr = head;
        while(ptr) {
            if(id == ptr -> product -> getId()) {
                cout << "Product with this ID already exists." << endl;
                return;
            }
            ptr = ptr -> next;
        }
        Product* prod = new Product(id, name, cat, price, qty);
        head = new InventoryNode(prod, head);
        productTrie->insert(name);
    }

    void updateProduct(int id, string name, string cat, double price, int qty) {
        InventoryNode* ptr = head;
        while(ptr && ptr -> product -> getId() != id) {
            ptr = ptr -> next;
        }
        if(!ptr) {return;}
        ptr -> product -> setName(name);
        ptr -> product -> setCategory(cat);
        ptr -> product -> setPrice(price);
        ptr -> product -> setQuantity(qty);
    }

    void removeProduct(int id) {
        if(!head) {return;}
        if(id == head -> product -> getId()) {
            InventoryNode* temp = head;
            head = head -> next;
            delete temp;
            return;
        }
        InventoryNode* ptr = head;
        while(ptr -> next && ptr -> next -> product -> getId() != id) {
            ptr = ptr -> next;
        }
        if(!ptr -> next) {return;}
        InventoryNode* temp = ptr -> next;
        ptr -> next = temp -> next;
        delete temp;
        cout << "Product with ID " << id << " removed from inventory." << endl;
    }

    Product* searchById(int id) {
        if(!head) {return nullptr;}
        InventoryNode* ptr = head;
        while(ptr) {
            if(ptr -> product -> getId() == id) {return ptr -> product;}
            ptr = ptr -> next;
        }
        return nullptr;
    }

    Product* searchByName(string name) {
        name = lower(name);
        if(!head) {return nullptr;}
        InventoryNode* ptr = head;
        while(ptr) {
            if(lower(ptr->product->getName()) == name) {return ptr -> product;}
            ptr = ptr -> next;
        }
        return nullptr;
    }

    void searchWithAutocomplete() {
        string prefix;
        cout << "Enter name prefix: ";
        cin >> prefix;
        productTrie->autocomplete(prefix);
    }


    void displayAllProducts() {
        if(!head) {cout << "Inventory is empty." << endl;}
        InventoryNode* ptr = head;
        while(ptr) {
            ptr -> product -> displayProduct();
            ptr = ptr -> next;
        }
    }

    void displayByCategory(string cat) {
        cat = lower(cat);
        if(!head) {
            cout << "Inventory is empty." << endl;
            return;
        }
        InventoryNode* ptr = head;
        bool found = false;
        while(ptr) {
            if(lower(ptr->product->getCategory()) == cat) {
                ptr->product->displayProduct();
                found = true;
            }
            ptr = ptr->next;
        }
        if(!found) {
            cout << "No products found in category: " << cat << endl;
        }
    }

    void viewLowStock(int low) {
        if(!head) {
            cout << "Inventory is empty." << endl;
            return;
        }
        InventoryNode* ptr = head;
        while(ptr) {
            if(ptr -> product -> getQuantity() < low) {
                cout << "Product: " << ptr -> product -> getName() << " | Remaining: " << ptr -> product -> getQuantity() << endl;
            }
            ptr = ptr->next;
        }
    }

    bool checkLowStock(int low) {
        if(!head) {
            return false;
        }
        InventoryNode* ptr = head;
        while(ptr) {
            if(ptr -> product -> getQuantity() < low) {
                return true;
            }
            ptr = ptr->next;
        }
        return false;
    }

    double calculateRevenue() {
        double revenue = 0.0;
        InventoryNode* ptr = head;

        while(ptr) {
            revenue += ptr->product->getPrice() * ptr->product->getSoldCount();
            ptr = ptr->next;
        }
        return revenue;
    }

    Product* getMostSoldProduct() {
        if(!head) return nullptr;

        InventoryNode* ptr = head;
        Product* best = nullptr;

        while(ptr) {
            if(!best || ptr->product->getSoldCount() > best->getSoldCount()) {
                best = ptr->product;
            }
            ptr = ptr->next;
        }
        return best;
    }

    ~Inventory() {
        while (head) {
            InventoryNode* temp = head;
            head = head->next;
            delete temp->product;
            delete temp;
        }
    }
};

class OrderQueue {
private:
    struct OrderNode {
        Cart* order;
        OrderNode* next;

        OrderNode(Cart* ord, OrderNode* n = nullptr) : order(ord), next(n) {}
    };

    OrderNode* first;
    OrderNode* last;

public:
    OrderQueue() : first(nullptr), last(nullptr) {}

    bool isEmpty() {return first == nullptr;}

    void enqueueOrder(Cart* order) {
        if(isEmpty()) {
            first = last = new OrderNode(order);
            return;
        }
        last -> next = new OrderNode(order);
        last = last -> next;
    }

    void dequeueOrder() {
        if(isEmpty()) {return;}
        OrderNode* temp = first;
        first = first -> next;
        delete temp -> order;
        if(!first) {last = nullptr;}
    }

    void displayAllOrders() {
        if (isEmpty()) {
            cout << "No pending orders.\n";
            return;
        }
        cout << "\n===== PENDING ORDERS =====\n";
        OrderNode* temp = first;
        int i = 1;
        while (temp) {
            cout << "\nOrder #" << i++ << endl;
            temp -> order -> viewCart();
            temp = temp -> next;
        }
        cout << "==========================\n";
    }

    Cart* viewNextOrder() {
        if (isEmpty()) {
            cout << "No pending orders." << endl;
            return nullptr;
        }
        cout << "\n===== NEXT ORDER IN QUEUE =====" << endl;
        return first->order;
    }

    ~OrderQueue() {
        while (!isEmpty()) {
            dequeueOrder();
        }
    }
};

class CategoryBST {
private:
    struct CategoryNode {
        string categoryName;
        CategoryNode* left;
        CategoryNode* right;

        CategoryNode(string name) : categoryName(name), left(nullptr), right(nullptr) {}
    };

    CategoryNode* root;

    void displayInOrder(CategoryNode* node) {
        if (!node) return;
        displayInOrder(node -> left);
        cout << node -> categoryName << endl;
        displayInOrder(node -> right);
    }

    void deleteTree(CategoryNode* node) {
        if(!node) return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }

public:
    CategoryBST() : root(nullptr) {}

    void insertCategory(string name) {
        name = lower(name);
        if(!root) {
            root = new CategoryNode(name);
            return;
        }
        CategoryNode* curr = root;
        CategoryNode* parent = nullptr;
        while(curr) {
            parent = curr;
            if(name < curr -> categoryName) {
                curr = curr -> left;
            }
            else if(name > curr -> categoryName) {
                curr = curr -> right;
            }
            else {
                cout << "Category " << name << " already exist." << endl;
                return;
            }
        }
        if(name < parent -> categoryName) {
            parent -> left = new CategoryNode(name);
        }
        else {
            parent -> right = new CategoryNode(name);
        }
    }

    CategoryNode* searchCategory(string name) {
        if(!root) {
            cout << "Category " << name << " not found." << endl;
            return nullptr;
        }
        CategoryNode* curr = root;
        while(curr) {
            if(curr -> categoryName == name) {
                return curr;
            }
            if(name < curr -> categoryName) {
                curr = curr -> left;
            }
            else {
                curr = curr -> right;
            }
        }
        cout << "Category " << name << " not found." << endl;
        return nullptr;
    }

    void displayCategories() {
        if(!root) {
            cout << "No categories yet." << endl;
            return;
        }
        cout << "===== CATEGORIES =====" << endl;
        displayInOrder(root);
        cout << "======================" << endl;

    }

    ~CategoryBST() {
        deleteTree(root);
    }
};

class Admin {
private:
    string adminPass;
    Inventory* inventory;
    OrderQueue* orderQueue;
    CategoryBST* categories;

public:
    Admin(string pass, Inventory* inv, OrderQueue* orders, CategoryBST* catbst) : adminPass(pass), inventory(inv), orderQueue(orders), categories(catbst) {}

    string getAdminPass() {return adminPass;}

    void addProduct(int id, string name, string cat, double price, int qty) {
        inventory -> addNewProduct(id, name, cat, price, qty);
        cout << "Product '" << name << "' added to inventory." << endl;
        if (!categories -> searchCategory(cat)) {
            categories -> insertCategory(cat);
            cout << "New category '" << cat << "' added to categories." << endl;
        }
    }

    void updateProduct(int id, string name, string cat, double price, int qty) {
        inventory -> updateProduct(id, name, cat, price, qty);
        if (!categories -> searchCategory(cat)) {
            categories -> insertCategory(cat);
            cout << "New category '" << cat << "' added to categories." << endl;
        }
        cout << "Product with ID " << id << " updated." << endl;
    }

    void removeProduct(int id) {
        inventory -> removeProduct(id);
    }

    void viewInventory() {
        cout << "\n======= INVENTORY =======" << endl;
        inventory -> displayAllProducts();
        cout << "=========================" << endl;
    }

    void viewLowStock(int low) {
        cout << "\n======= !LOW STOCK! =======" << endl;
        inventory -> viewLowStock(low);
        cout << "=========================" << endl;
    }

    void processOrders() {
        if (orderQueue->isEmpty()) {
            cout << "No pending orders.\n";
            return;
        }
        orderQueue->displayAllOrders();
        Cart* order = orderQueue->viewNextOrder();
        order->viewCart();
        int choice;
        cout << "1. Approve Order\n";
        cout << "2. Reject Order\n";
        cout << "3. Return\n";
        cout << "Choose: ";
        cin >> choice;
        if (choice == 1) {
            order->applyOrderToInventory();
            orderQueue->dequeueOrder();
            cout << "Order Approved.\n";
        }
        else if (choice == 2) {
            orderQueue->dequeueOrder();
            cout << "Order Rejected.\n";
        }
        else {
            cout << "Returning...\n";
        }
    }

    void viewReports() {
        cout << "\n===== REPORTS =====\n";
        cout << "\n--- TOTAL REVENUE ---\n";
        cout << "Revenue: $" << inventory->calculateRevenue() << endl;
        cout << "\n--- MOST SOLD PRODUCT ---\n";
        Product* best = inventory->getMostSoldProduct();
        if(best && best->getSoldCount() > 0) {
            cout << "Name: " << best->getName() << endl;
            cout << "Sold: " << best->getSoldCount() << endl;
        } else {
            cout << "No sales yet.\n";
        }
        cout << "\n--- LOW STOCK PRODUCTS ---\n";
        inventory->viewLowStock(20);
    }
};

class Customer {
private:
    Cart* cart;
    int ordersCount;
    string phoneNumber;

public:
    Customer(string number) : ordersCount(0), phoneNumber(number) {
        cart = new Cart();
    }

    Cart* getCart() { return cart;}
    int getOrdersCount() { return ordersCount; }
    string getPhone() { return phoneNumber; }

    void addToCart(Product* prod, int qty) {
        if(prod) {
            cart -> addItem(prod, qty);
            cout << prod -> getName() << " added to cart" << endl;
        }
    }

    void removeFromCart(int id) {
        cart -> removeItem(id);
        cout << " removed successfully" << endl;
    }

    void updateCartItem(int id, int qty) {
        cart -> updateItemQuantity(id, qty);
    }

    void viewCart() {
        cart -> viewCart();
    }

    void checkout(OrderQueue* orders) {
        double total = cart -> calculateTotal();
        if(total == 0) {
            cout << "Cart is empty." << endl;
            return;
        }
        double taxRate = 0.14;
        double tax = total * taxRate;
        double finalTotal = total + tax;
        cout << "\n===== BILL =====" << endl;
        cart -> viewCart();
        cout << "Subtotal: $" << total << endl;
        cout << "Tax (" << taxRate*100 << "%): $" << tax << endl;
        cout << "Total: $" << finalTotal << endl;
        char confirm;
        cout << "Confirm order? (y/n): ";
        cin >> confirm;

        if(confirm == 'y' || confirm == 'Y') {
            Cart* orderCopy = new Cart(*getCart());
            orders->enqueueOrder(cart);
            cart = new Cart();
            ordersCount++;
            cout << "Order placed successfully!\n";
        } else {
            cout << "Checkout cancelled.\n";
        }
    }
};

void adminMenu(Admin& admin, Inventory& inv) {
    int choice;
    while(true) {
        cout << "\n===== ADMIN MENU =====" << endl;
        if(inv.checkLowStock(20)) {
            cout << "\n!!! CHECK LOW STOCK !!!" << endl;
        }
        cout << "\n1. Add Product" << endl;
        cout << "2. Update Product" << endl;
        cout << "3. Remove Product" << endl;
        cout << "4. View Inventory" << endl;
        cout << "5. View Low Stock Products" << endl;
        cout << "6. Process Orders\n";
        cout << "7. View Reports\n";
        cout << "0. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch(choice) {
            case 1: {
                int id, qty;
                double price;
                string name, cat;
                cout << "Enter product ID: ";
                cin >> id;
                cin.ignore();
                cout << "Enter product name: ";
                getline(cin, name);
                cout << "Enter product category: ";
                getline(cin, cat);
                cout << "Enter price: ";
                cin >> price;
                cout << "Enter quantity: ";
                cin >> qty;
                name = lower(name);
                cat = lower(cat);
                admin.addProduct(id, name, cat, price, qty);
                break;
            }
            case 2: {
                int id, qty;
                double price;
                string name, cat;
                cout << "Enter product ID to update: ";
                cin >> id;
                if(inv.searchById(id)) {
                    cin.ignore();
                    cout << "Enter new name: ";
                    getline(cin, name);
                    cout << "Enter new category: ";
                    getline(cin, cat);
                    cout << "Enter new price: ";
                    cin >> price;
                    cout << "Enter new quantity: ";
                    cin >> qty;
                    name = lower(name);
                    cat = lower(cat);
                    admin.updateProduct(id, name, cat, price, qty);
                }
                else {
                    cout << "ID " << id << " not found." << endl;
                }
                break;
            }
            case 3: {
                int id;
                cout << "Enter product ID to remove: ";
                cin >> id;
                if(inv.searchById(id)) {
                    admin.removeProduct(id);
                }
                else {
                    cout << "ID " << id << " not found." << endl;
                }
                break;
            }
            case 4:
                admin.viewInventory();
                break;
            case 5:
                int lower;
                cout << "Enter low threshold: ";
                cin >> lower;
                admin.viewLowStock(lower);
                break;
            case 6:
                admin.processOrders();
                break;
            case 7:
                admin.viewReports();
                break;
            case 0:
                cout << "Exiting admin menu...\n";
                return;
            default:
                cout << "Invalid choice." << endl;
                break;
        }
    }
}

void customerMenu(Customer& customer, Inventory* inventory, CategoryBST* categories, OrderQueue* orders) {
    int choice;
    while(true) {
        cout << "\n===== CUSTOMER MENU =====" << endl;
        cout << "Phone: " << customer.getPhone() << " | Orders Placed: " << customer.getOrdersCount() << endl;
        cout << "1. Browse Products by Category" << endl;
        cout << "2. Search Product by Name" << endl;
        cout << "3. View Cart" << endl;
        cout << "4. Add Item to Cart" << endl;
        cout << "5. Remove Item from Cart" << endl;
        cout << "6. Update Item Quantity" << endl;
        cout << "7. Checkout" << endl;
        cout << "8. Undo Last Action" << endl;
        cout << "9. Search Product (Autocomplete)" << endl;
        cout << "0. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch(choice) {
            case 1: {
                categories -> displayCategories();
                string cat;
                cin.ignore();
                cout << "Enter category to browse: ";
                getline(cin, cat);
                cat = lower(cat);
                inventory -> displayByCategory(cat);
                break;
            }
            case 2: {
                string name;
                cin.ignore();
                cout << "Enter product name to search: ";
                getline(cin, name);
                name = lower(name);
                Product* prod = inventory-> searchByName(name);
                if(prod) {
                    prod -> displayProduct();
                }
                else {
                    cout << "Product not found." << endl;
                }
                break;
            }
            case 3:
                customer.viewCart();
                break;
            case 4: {
                int id, qty;
                cout << "Enter product ID and quantity: ";
                cin >> id >> qty;
                Product* prod = inventory->searchById(id);
                if(prod) {
                    customer.addToCart(prod, qty);
                }
                else cout << "Product not found.\n";
                break;
            }
            case 5: {
                int id;
                cout << "Enter product ID to remove: ";
                cin >> id;
                Product* prod = inventory->searchById(id);
                if(prod) {
                    customer.removeFromCart(id);
                }
                else cout << "Product not found.\n";
                break;
            }
            case 6: {
                int id, qty;
                cout << "Enter product ID and new quantity: ";
                cin >> id >> qty;
                customer.updateCartItem(id, qty);
                break;
            }
            case 7: {
                customer.checkout(orders);
                break;
            }
            case 8: {
                customer.getCart()->undoLastAction();
                break;
            }
            case 9: {
                inventory->searchWithAutocomplete();
                break;
            }
            case 0: {
                cout << "Exiting customer menu..." << endl;
                return;
            }
            default:
                cout << "Invalid choice." << endl;
        }
    }
}

int main() {

    Inventory inventory;
    OrderQueue orderQueue;
    CategoryBST categoryTree;
    Admin admin("123", &inventory, &orderQueue, &categoryTree);
    Cart tempCart;
    int maxCustomers = 10000;
    Customer* customers[maxCustomers];
    int customerCount = 0;

    // Test data
    admin.addProduct(1, "Milk", "Dairy", 20, 50);
    admin.addProduct(2, "Bread", "Bakery", 10, 30);
    admin.addProduct(3, "Rice", "Grains", 25, 100);

    int choice;

    while (true) {

        cout << "\n==============================\n";
        cout << "   KAKS Supermarket System   \n";
        cout << "==============================\n";
        cout << "1. Admin" << endl;
        cout << "2. Customer" << endl;
        cout << "0. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {

            case 1: {
                string pass;
                cout << "Enter Admin Password: ";
                cin >> pass;
                if (admin.getAdminPass() == pass) {
                    cout << "Login Successful!" << endl;;
                    adminMenu(admin, inventory);
                } else {
                    cout << "Wrong Password!" << endl;
                }
                break;
            }

            case 2: {
                string phone;
                cin.ignore();
                cout << "Enter your phone number: ";
                getline(cin, phone);
                Customer* currentCustomer = nullptr;
                for(int i = 0; i < customerCount; i++) {
                    if(customers[i]->getPhone() == phone) {
                        currentCustomer = customers[i];
                        break;
                    }
                }
                if(!currentCustomer) {
                    if(customerCount < maxCustomers) {
                        currentCustomer = new Customer(phone);
                        customers[customerCount++] = currentCustomer;
                    } 
                    else {
                        cout << "Customer limit reached!" << endl;
                        break;
                    }
                }
                customerMenu(*currentCustomer, &inventory, &categoryTree, &orderQueue);
                break;
            }

            case 0: {
                cout << "Exiting program..." << endl;
                return 0;
            }

            default: {
                cout << "Invalid choice." << endl;
                break;
            }
        }
    }

    for(int i = 0; i < customerCount; i++) {
        delete customers[i];
    }
    return 0;
}
