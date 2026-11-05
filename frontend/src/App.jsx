import { useEffect, useMemo, useState } from "react";
import "./App.css";

const API = "http://localhost:5000";
const categories = ["All", "Burgers", "Pizza", "Snacks", "Drinks"];
const ORDER_STATUSES = ["Pending", "Preparing", "Delivered"];

function App() {
  const [foods, setFoods] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState("");
  const [activeCategory, setActiveCategory] = useState("All");
  const [search, setSearch] = useState("");
  const [cart, setCart] = useState([]);
  const [showCart, setShowCart] = useState(false);
  const [customerName, setCustomerName] = useState("");
  const [customerPhone, setCustomerPhone] = useState("");
  const [orderMessage, setOrderMessage] = useState("");
  const [submitting, setSubmitting] = useState(false);
  const [showAdmin, setShowAdmin] = useState(false);
  const [adminToken, setAdminToken] = useState("");
  const [adminTab, setAdminTab] = useState("orders");
  const [adminOrders, setAdminOrders] = useState([]);
  const [adminLoading, setAdminLoading] = useState(false);
  const [adminError, setAdminError] = useState("");
  const [showMyOrders, setShowMyOrders] = useState(false);

  const visibleFoods = useMemo(() => {
    return foods.filter((food) => {
      const matchesCategory =
        activeCategory === "All" || food.category === activeCategory;

      const matchesSearch = food.name
        .toLowerCase()
        .includes(search.toLowerCase());

      return matchesCategory && matchesSearch;
    });
  }, [foods, activeCategory, search]);

  const cartCount = cart.reduce(
    (total, item) => total + item.quantity,
    0
  );

  const cartTotal = cart.reduce(
    (total, item) => total + item.price * item.quantity,
    0
  );

  async function loadFoods() {
    setLoading(true);

    try {
      const response = await fetch(`${API}/api/food`);

      if (!response.ok) {
        throw new Error("Failed to load food menu");
      }

      const data = await response.json();
      setFoods(Array.isArray(data) ? data : []);
      setError("");
    } catch {
      setError("Unable to load the menu. Please check the backend.");
    } finally {
      setLoading(false);
    }
  }

  useEffect(() => {
    loadFoods();
  }, []);

  function addToCart(food) {
    setOrderMessage("");

    setCart((currentCart) => {
      const existing = currentCart.find((item) => item.id === food.id);

      if (existing) {
        return currentCart.map((item) =>
          item.id === food.id
            ? { ...item, quantity: item.quantity + 1 }
            : item
        );
      }

      return [...currentCart, { ...food, quantity: 1 }];
    });
  }

  function changeQuantity(foodId, amount) {
    setCart((currentCart) =>
      currentCart
        .map((item) =>
          item.id === foodId
            ? { ...item, quantity: item.quantity + amount }
            : item
        )
        .filter((item) => item.quantity > 0)
    );
  }

  // Place the order using the backend API
  async function placeOrder() {
    if (!customerName.trim()) {
      setOrderMessage("Please enter your name.");
      return;
    }

    if (cart.length === 0 || submitting) {
      return;
    }

    setSubmitting(true);
    setOrderMessage("");

    try {
      const body = {
        customerName: customerName.trim(),
        items: cart.map((item) => ({
          id: item.id,
          quantity: item.quantity,
        })),
      };

      if (customerPhone.trim()) {
        body.phone = customerPhone.trim();
      }

      const response = await fetch(`${API}/api/orders`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(body),
      });

      const data = await response.json();

      if (!response.ok) {
        throw new Error(data.message || "Unable to place order.");
      }

      setOrderMessage(
        `Order #${data.order.id} placed successfully! Total: ₹${data.order.total}`
      );

      setCart([]);
      setCustomerName("");
      setCustomerPhone("");
    } catch (error) {
      setOrderMessage(
        error.message || "Could not connect to the backend."
      );
    } finally {
      setSubmitting(false);
    }
  }

  // Sign in as admin; stores the server-issued token used by admin endpoints.
  async function adminSignIn(username, password) {
    const response = await fetch(`${API}/api/admin/login`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ username, password }),
    });

    const data = await response.json();

    if (!response.ok || !data.success) {
      throw new Error(data.message || "Invalid username or password.");
    }

    setAdminToken(data.token);
    setAdminTab("orders");
    fetchAdminOrders(data.token);
  }

  function adminSignOut() {
    setAdminToken("");
    setAdminOrders([]);
    setAdminError("");
  }

  // Fetch all orders for the admin (requires the admin token)
  async function fetchAdminOrders(token = adminToken) {
    if (!token) {
      return;
    }

    setAdminLoading(true);
    setAdminError("");

    try {
      const response = await fetch(`${API}/api/orders`, {
        headers: { Authorization: `Bearer ${token}` },
      });
      const data = await response.json();

      if (response.status === 401) {
        setAdminToken("");
        throw new Error("Session expired. Please sign in again.");
      }

      if (!response.ok || !data.success) {
        throw new Error(data.message || "Failed to load orders.");
      }

      setAdminOrders(data.orders);
    } catch (error) {
      setAdminError(error.message || "Could not load orders.");
    } finally {
      setAdminLoading(false);
    }
  }

  // Update an order's status (requires the admin token)
  async function updateOrderStatus(orderId, status) {
    try {
      const response = await fetch(
        `${API}/api/orders/${orderId}/status`,
        {
          method: "PATCH",
          headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${adminToken}`,
          },
          body: JSON.stringify({ status }),
        }
      );

      const data = await response.json();

      if (response.status === 401) {
        setAdminToken("");
        throw new Error("Session expired. Please sign in again.");
      }

      if (!response.ok || !data.success) {
        throw new Error(data.message || "Failed to update status.");
      }

      setAdminOrders((currentOrders) =>
        currentOrders.map((order) =>
          order.id === orderId ? data.order : order
        )
      );
    } catch (error) {
      setAdminError(error.message || "Could not update order status.");
    }
  }

  // Admin food CRUD -----------------------------------------------------------

  async function adminRequest(path, options = {}) {
    const response = await fetch(`${API}${path}`, {
      ...options,
      headers: {
        "Content-Type": "application/json",
        Authorization: `Bearer ${adminToken}`,
        ...(options.headers || {}),
      },
    });

    const data = await response.json().catch(() => ({}));

    if (response.status === 401) {
      setAdminToken("");
      throw new Error("Session expired. Please sign in again.");
    }

    if (!response.ok || data.success === false) {
      throw new Error(data.message || "Request failed.");
    }

    return data;
  }

  async function addFood(food) {
    const data = await adminRequest("/api/food", {
      method: "POST",
      body: JSON.stringify(food),
    });

    await loadFoods();
    return data.food;
  }

  async function updateFood(id, food) {
    const data = await adminRequest(`/api/food/${id}`, {
      method: "PUT",
      body: JSON.stringify(food),
    });

    await loadFoods();
    return data.food;
  }

  async function deleteFood(id) {
    await adminRequest(`/api/food/${id}`, { method: "DELETE" });
    await loadFoods();
  }

  if (showAdmin) {
    return (
      <div className="app-shell">
        <aside className="sidebar">
          <div className="brand">
            <div className="brand-icon">🍽️</div>
            <div>
              <h2>CampusBite</h2>
              <p>CANTEEN MANAGEMENT</p>
            </div>
          </div>

          <p className="nav-label">MANAGEMENT</p>

          <button
            className={`nav-item ${adminTab === "orders" ? "active" : ""}`}
            onClick={() => {
              setAdminTab("orders");
              fetchAdminOrders();
            }}
          >
            📋 <span>Admin Orders</span>
          </button>

          <button
            className={`nav-item ${adminTab === "menu" ? "active" : ""}`}
            onClick={() => setAdminTab("menu")}
          >
            🍔 <span>Manage Menu</span>
          </button>

          <div className="sidebar-bottom">
            {adminToken && (
              <button className="nav-item" onClick={adminSignOut}>
                ↩ <span>Sign out</span>
              </button>
            )}

            <div className="help-card">
              <span>💡</span>
              <h3>Manage orders</h3>
              <p>Track and update customer orders.</p>
            </div>
            <p className="sidebar-footer">Made for campus life ❤️</p>
          </div>
        </aside>

        {adminToken ? (
          adminTab === "orders" ? (
            <AdminOrders
              orders={adminOrders}
              loading={adminLoading}
              error={adminError}
              onRefresh={() => fetchAdminOrders()}
              onUpdateStatus={updateOrderStatus}
              onBack={() => setShowAdmin(false)}
            />
          ) : (
            <AdminMenu
              foods={foods}
              loading={loading}
              error={error}
              onRefresh={loadFoods}
              onAddFood={addFood}
              onUpdateFood={updateFood}
              onDeleteFood={deleteFood}
              onBack={() => setShowAdmin(false)}
            />
          )
        ) : (
          <AdminLogin onSubmit={adminSignIn} onBack={() => setShowAdmin(false)} />
        )}
      </div>
    );
  }

  return (
    <div className="app-shell">
      <aside className="sidebar">
        <div className="brand">
          <div className="brand-icon">🍽️</div>
          <div>
            <h2>CampusBite</h2>
            <p>CANTEEN MANAGEMENT</p>
          </div>
        </div>

        <p className="nav-label">MENU</p>
        <button className="nav-item active">
          ⌂ <span>Explore Food</span>
        </button>

        <button className="nav-item" onClick={() => setShowCart(true)}>
          🛒 <span>My Cart</span>
          {cartCount > 0 && <b className="nav-count">{cartCount}</b>}
        </button>

        <button className="nav-item" onClick={() => setShowMyOrders(true)}>
          📦 <span>My Orders</span>
        </button>

        <button
          className={`nav-item ${showAdmin ? "active" : ""}`}
          onClick={() => {
            setShowAdmin(true);
          }}
        >
          📋 <span>Admin</span>
        </button>

        <div className="sidebar-bottom">
          <div className="help-card">
            <span>💡</span>
            <h3>Hungry?</h3>
            <p>Your favourite campus meals are just a few clicks away.</p>
          </div>
          <p className="sidebar-footer">Made for campus life ❤️</p>
        </div>
      </aside>

      <main className="main-content">
        <header className="topbar">
          <div>
            <p className="eyebrow">YOUR CAMPUS, YOUR FOOD</p>
            <h1>
              Good food. <span>Good mood.</span>
            </h1>
          </div>

          <button className="cart-button" onClick={() => setShowCart(true)}>
            🛒 My Cart <span>{cartCount}</span>
          </button>
        </header>

        <section className="welcome-banner">
          <div className="banner-copy">
            <span className="banner-tag">FRESHLY MADE, EVERY DAY</span>
            <h2>
              Cravings don't
              <br />
              wait. Neither should you.
            </h2>
            <p>Order your favourites and enjoy more of your break.</p>

            <button
              className="banner-button"
              onClick={() =>
                document.getElementById("food-menu")?.scrollIntoView({
                  behavior: "smooth",
                })
              }
            >
              Explore menu <span>→</span>
            </button>
          </div>

          <div className="banner-art" aria-hidden="true">
            <span className="art-circle">🍔</span>
            <span className="art-small art-one">🍟</span>
            <span className="art-small art-two">🥤</span>
            <span className="art-spark">✳</span>
          </div>
        </section>

        <section className="menu-section" id="food-menu">
          <div className="section-heading">
            <div>
              <p className="eyebrow">MADE TO SATISFY</p>
              <h2>What are you craving?</h2>
            </div>

            <div className="search-box">
              <span>⌕</span>
              <input
                value={search}
                onChange={(event) => setSearch(event.target.value)}
                placeholder="Search food..."
                aria-label="Search food"
              />
            </div>
          </div>

          <div className="category-list">
            {categories.map((category) => (
              <button
                key={category}
                className={`category-button ${
                  activeCategory === category ? "selected" : ""
                }`}
                onClick={() => setActiveCategory(category)}
              >
                {category}
              </button>
            ))}
          </div>

          {loading && <p>Loading menu...</p>}

          {error && <p>{error}</p>}

          <div className="food-grid">
            {!loading &&
              !error &&
              visibleFoods.map((food) => (
                <article className="food-card" key={food.id}>
                  <div
                    className="food-art"
                    style={{ backgroundColor: food.color }}
                  >
                    <span>{food.emoji}</span>
                    <span className="food-category">{food.category}</span>
                  </div>

                  <div className="food-details">
                    <div className="food-title-row">
                      <h3>{food.name}</h3>
                      <strong>₹{food.price}</strong>
                    </div>

                    <p>{food.description}</p>

                    <button
                      className="add-button"
                      onClick={() => addToCart(food)}
                    >
                      + Add to cart
                    </button>
                  </div>
                </article>
              ))}
          </div>

          {!loading && !error && visibleFoods.length === 0 && (
            <div className="empty-state">
              <span>🔎</span>
              <h3>No food found</h3>
              <p>Try a different name or category.</p>
            </div>
          )}
        </section>

        <footer className="page-footer">
          <span>CampusBite © 2026</span>
          <span>Fresh food, happier breaks.</span>
        </footer>
      </main>

      {showCart && (
        <div
          className="modal-backdrop"
          onClick={() => setShowCart(false)}
        >
          <section
            className="cart-panel"
            onClick={(event) => event.stopPropagation()}
            role="dialog"
            aria-modal="true"
            aria-label="Your shopping cart"
          >
            <div className="cart-heading">
              <div>
                <p className="eyebrow">YOUR ORDER</p>
                <h2>My Cart ({cartCount})</h2>
              </div>

              <button
                className="close-button"
                onClick={() => setShowCart(false)}
                aria-label="Close cart"
              >
                ×
              </button>
            </div>

            {orderMessage && (
              <p role="status" className="order-message">
                {orderMessage}
              </p>
            )}

            {cart.length === 0 ? (
              <div className="empty-cart">
                <span>🛒</span>
                <h3>
                  {orderMessage.includes("placed successfully")
                    ? "Order confirmed!"
                    : "Your cart is empty"}
                </h3>
                <p>
                  {orderMessage.includes("placed successfully")
                    ? "Thank you! Your order has been submitted."
                    : "Add something delicious from the menu."}
                </p>

                <button
                  className="add-button"
                  onClick={() => setShowCart(false)}
                >
                  Explore food
                </button>
              </div>
            ) : (
              <>
                <div className="cart-items">
                  {cart.map((item) => (
                    <div className="cart-item" key={item.id}>
                      <div className="cart-item-emoji">{item.emoji}</div>

                      <div className="cart-item-info">
                        <h3>{item.name}</h3>
                        <p>₹{item.price} each</p>

                        <div className="quantity-control">
                          <button
                            onClick={() => changeQuantity(item.id, -1)}
                          >
                            −
                          </button>
                          <span>{item.quantity}</span>
                          <button
                            onClick={() => changeQuantity(item.id, 1)}
                          >
                            +
                          </button>
                        </div>
                      </div>

                      <strong>₹{item.price * item.quantity}</strong>
                    </div>
                  ))}
                </div>

                <div className="cart-total">
                  <span>Total</span>
                  <strong>₹{cartTotal}</strong>
                </div>

                <div className="checkout-form">
                  <label htmlFor="customer-name">Your name</label>

                  <input
                    id="customer-name"
                    type="text"
                    value={customerName}
                    onChange={(event) => setCustomerName(event.target.value)}
                    placeholder="Enter your name"
                  />

                  <label htmlFor="customer-phone">
                    Phone number <span className="field-hint">(optional)</span>
                  </label>

                  <input
                    id="customer-phone"
                    type="tel"
                    inputMode="numeric"
                    value={customerPhone}
                    onChange={(event) => setCustomerPhone(event.target.value)}
                    placeholder="e.g. 9876543210"
                  />

                  <button
                    className="checkout-button"
                    onClick={placeOrder}
                    disabled={submitting || cart.length === 0}
                  >
                    {submitting
                      ? "Placing order..."
                      : `Place order · ₹${cartTotal}`}
                  </button>
                </div>

                <p className="checkout-note">
                  Add your phone number to track this order under “My Orders”.
                </p>
              </>
            )}
          </section>
        </div>
      )}

      {showMyOrders && (
        <MyOrders onClose={() => setShowMyOrders(false)} onBrowse={() => setShowCart(true)} />
      )}
    </div>
  );
}

function AdminLogin({ onSubmit, onBack }) {
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [error, setError] = useState("");
  const [busy, setBusy] = useState(false);

  async function handleSubmit(event) {
    event.preventDefault();

    if (busy) {
      return;
    }

    setBusy(true);
    setError("");

    try {
      await onSubmit(username.trim(), password);
    } catch (submitError) {
      setError(submitError.message || "Sign in failed.");
    } finally {
      setBusy(false);
    }
  }

  return (
    <main className="main-content">
      <header className="topbar">
        <div>
          <p className="eyebrow">CAMPUSBITE MANAGEMENT</p>
          <h1>Admin <span>sign in.</span></h1>
        </div>

        <button className="cart-button" onClick={onBack}>
          ← Back to menu
        </button>
      </header>

      <section className="menu-section">
        <div className="auth-card">
          <div className="empty-state" style={{ padding: "0 0 22px" }}>
            <span>🔐</span>
            <h3>Restricted area</h3>
            <p>Sign in to manage orders and the menu.</p>
          </div>

          <form className="checkout-form" onSubmit={handleSubmit}>
            {error && (
              <p role="alert" className="order-message error">
                {error}
              </p>
            )}

            <label htmlFor="admin-username">Username</label>
            <input
              id="admin-username"
              type="text"
              autoComplete="username"
              value={username}
              onChange={(event) => setUsername(event.target.value)}
              placeholder="Enter username"
            />

            <label htmlFor="admin-password">Password</label>
            <input
              id="admin-password"
              type="password"
              autoComplete="current-password"
              value={password}
              onChange={(event) => setPassword(event.target.value)}
              placeholder="Enter password"
            />

            <button
              className="checkout-button"
              type="submit"
              disabled={busy || !username.trim() || !password}
            >
              {busy ? "Signing in..." : "Sign in"}
            </button>
          </form>
        </div>
      </section>
    </main>
  );
}

function MyOrders({ onClose, onBrowse }) {
  const [name, setName] = useState("");
  const [phone, setPhone] = useState("");
  const [session, setSession] = useState(null);
  const [orders, setOrders] = useState({ pending: [], history: [] });
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);
  const [busy, setBusy] = useState(false);

  async function loadOrders(token, customerId) {
    setLoading(true);
    setError("");

    try {
      const response = await fetch(`${API}/api/customers/${customerId}/orders`, {
        headers: { Authorization: `Bearer ${token}` },
      });
      const data = await response.json();

      if (!response.ok || !data.success) {
        throw new Error(data.message || "Could not load your orders.");
      }

      setOrders({
        pending: Array.isArray(data.pending) ? data.pending : [],
        history: Array.isArray(data.history) ? data.history : [],
      });
    } catch (loadError) {
      setError(loadError.message || "Could not load your orders.");
    } finally {
      setLoading(false);
    }
  }

  async function handleLookup(event) {
    event.preventDefault();

    if (busy) {
      return;
    }

    setBusy(true);
    setError("");

    try {
      const response = await fetch(`${API}/api/customers/login`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ name: name.trim(), phone: phone.trim() }),
      });
      const data = await response.json();

      if (!response.ok || !data.success) {
        throw new Error(data.message || "Could not find your orders.");
      }

      setSession({ token: data.token, customerId: data.customer.id });
      await loadOrders(data.token, data.customer.id);
    } catch (lookupError) {
      setError(lookupError.message || "Could not find your orders.");
    } finally {
      setBusy(false);
    }
  }

  function renderOrders(list) {
    return list.map((order) => (
      <article className="order-row" key={order.id}>
        <div className="order-row-head">
          <h3>Order #{order.id}</h3>
          <span className={`status-pill status-${order.status.toLowerCase()}`}>
            {order.status}
          </span>
        </div>
        <p className="order-row-items">
          {order.items.map((item) => `${item.name} × ${item.quantity}`).join(", ")}
        </p>
        <div className="order-row-foot">
          <span>{order.items.reduce((sum, item) => sum + item.quantity, 0)} items</span>
          <strong>₹{order.total}</strong>
        </div>
      </article>
    ));
  }

  const hasOrders = orders.pending.length + orders.history.length > 0;

  return (
    <div className="modal-backdrop" onClick={onClose}>
      <section
        className="cart-panel"
        onClick={(event) => event.stopPropagation()}
        role="dialog"
        aria-modal="true"
        aria-label="Your orders"
      >
        <div className="cart-heading">
          <div>
            <p className="eyebrow">TRACK YOUR FOOD</p>
            <h2>My Orders</h2>
          </div>

          <button className="close-button" onClick={onClose} aria-label="Close orders">
            ×
          </button>
        </div>

        {!session ? (
          <form className="checkout-form" onSubmit={handleLookup}>
            <p className="order-row-items">
              Enter the name and phone number you used when ordering.
            </p>

            {error && (
              <p role="alert" className="order-message error">
                {error}
              </p>
            )}

            <label htmlFor="lookup-name">Your name</label>
            <input
              id="lookup-name"
              type="text"
              value={name}
              onChange={(event) => setName(event.target.value)}
              placeholder="Enter your name"
            />

            <label htmlFor="lookup-phone">Phone number</label>
            <input
              id="lookup-phone"
              type="tel"
              inputMode="numeric"
              value={phone}
              onChange={(event) => setPhone(event.target.value)}
              placeholder="e.g. 9876543210"
            />

            <button
              className="checkout-button"
              type="submit"
              disabled={busy || !name.trim() || !phone.trim()}
            >
              {busy ? "Looking up..." : "View my orders"}
            </button>
          </form>
        ) : (
          <>
            <div className="my-orders-toolbar">
              <span>
                Signed in as <strong>{name.trim()}</strong>
              </span>
              <button className="add-button tool" onClick={() => loadOrders(session.token, session.customerId)}>
                Refresh
              </button>
            </div>

            {loading && <p>Loading orders...</p>}
            {error && (
              <p role="alert" className="order-message error">
                {error}
              </p>
            )}

            {!loading && !error && !hasOrders && (
              <div className="empty-cart">
                <span>📦</span>
                <h3>No orders yet</h3>
                <p>Place an order and it will show up here.</p>
                <button className="add-button" onClick={onBrowse}>
                  Explore food
                </button>
              </div>
            )}

            {!loading && !error && orders.pending.length > 0 && (
              <div className="order-group">
                <p className="nav-label">IN PROGRESS</p>
                {renderOrders(orders.pending)}
              </div>
            )}

            {!loading && !error && orders.history.length > 0 && (
              <div className="order-group">
                <p className="nav-label">HISTORY</p>
                {renderOrders(orders.history)}
              </div>
            )}
          </>
        )}
      </section>
    </div>
  );
}

function AdminOrders({ orders, loading, error, onRefresh, onUpdateStatus, onBack }) {
  return (
    <main className="main-content">
      <header className="topbar">
        <div>
          <p className="eyebrow">CAMPUSBITE MANAGEMENT</p>
          <h1>Manage <span>Orders.</span></h1>
        </div>

        <button className="cart-button" onClick={onBack}>
          ← Back to menu
        </button>
      </header>

      <section className="menu-section">
        <div className="section-heading">
          <div>
            <p className="eyebrow">ORDER MANAGEMENT</p>
            <h2>All orders</h2>
          </div>

          <button className="add-button tool" onClick={onRefresh}>
            Refresh orders
          </button>
        </div>

        {loading && <p>Loading orders...</p>}
        {error && <p role="alert">{error}</p>}

        {!loading && !error && orders.length === 0 && (
          <div className="empty-state">
            <span>📋</span>
            <h3>No orders yet</h3>
            <p>Customer orders will appear here.</p>
          </div>
        )}

        <div className="food-grid">
          {orders.map((order) => (
            <article className="food-card" key={order.id}>
              <div className="food-details">
                <div className="food-title-row">
                  <h3>Order #{order.id}</h3>
                  <strong>₹{order.total}</strong>
                </div>

                <p><strong>Customer:</strong> {order.customerName}</p>
                <p>
                  <strong>Items:</strong>{" "}
                  {order.items
                    .map((item) => `${item.name} × ${item.quantity}`)
                    .join(", ")}
                </p>
                <p>
                  <strong>Status:</strong> {order.status}
                </p>

                <label htmlFor={`order-status-${order.id}`}>
                  Change status
                </label>

                <select
                  id={`order-status-${order.id}`}
                  value={order.status}
                  disabled={order.status === "Delivered"}
                  onChange={(event) =>
                    onUpdateStatus(order.id, event.target.value)
                  }
                >
                  {ORDER_STATUSES.map((status) => (
                    <option key={status} value={status}>
                      {status}
                    </option>
                  ))}
                </select>
              </div>
            </article>
          ))}
        </div>
      </section>
    </main>
  );
}

function AdminMenu({ foods, loading, error, onRefresh, onAddFood, onUpdateFood, onDeleteFood, onBack }) {
  const emptyForm = {
    id: "",
    name: "",
    category: "Snacks",
    price: "",
    emoji: "",
    description: "",
    color: "#F2F2F2",
    available: true,
  };

  const [form, setForm] = useState(emptyForm);
  const [editingId, setEditingId] = useState(null);
  const [message, setMessage] = useState("");
  const [formError, setFormError] = useState("");
  const [busy, setBusy] = useState(false);

  function update(field, value) {
    setForm((current) => ({ ...current, [field]: value }));
  }

  function startEdit(food) {
    setEditingId(food.id);
    setForm({
      id: String(food.id),
      name: food.name,
      category: food.category,
      price: String(food.price),
      emoji: food.emoji || "",
      description: food.description || "",
      color: food.color || "#F2F2F2",
      available: Boolean(food.available),
    });
    setFormError("");
    setMessage("");
  }

  function resetForm() {
    setEditingId(null);
    setForm(emptyForm);
    setFormError("");
  }

  async function handleSubmit(event) {
    event.preventDefault();

    if (busy) {
      return;
    }

    setFormError("");
    setMessage("");

    const price = Number(form.price);

    if (!form.name.trim()) {
      setFormError("Food name is required.");
      return;
    }

    if (!Number.isFinite(price) || price < 0) {
      setFormError("Enter a valid price.");
      return;
    }

    const payload = {
      name: form.name.trim(),
      category: form.category.trim() || "Snacks",
      price,
      emoji: form.emoji,
      description: form.description,
      color: form.color,
      available: form.available,
    };

    setBusy(true);

    try {
      if (editingId) {
        await onUpdateFood(editingId, payload);
        setMessage(`Updated “${payload.name}”.`);
        resetForm();
      } else {
        const id = Number(form.id);

        if (!Number.isInteger(id) || id <= 0) {
          setFormError("Enter a positive numeric id.");
          setBusy(false);
          return;
        }

        await onAddFood({ id, ...payload });
        setMessage(`Added “${payload.name}”.`);
        resetForm();
      }
    } catch (submitError) {
      setFormError(submitError.message || "Request failed.");
    } finally {
      setBusy(false);
    }
  }

  async function handleDelete(food) {
    if (busy) {
      return;
    }

    if (!window.confirm(`Remove “${food.name}” from the menu?`)) {
      return;
    }

    setFormError("");
    setMessage("");
    setBusy(true);

    try {
      await onDeleteFood(food.id);
      setMessage(`Removed “${food.name}”.`);

      if (editingId === food.id) {
        resetForm();
      }
    } catch (deleteError) {
      setFormError(deleteError.message || "Could not remove item.");
    } finally {
      setBusy(false);
    }
  }

  return (
    <main className="main-content">
      <header className="topbar">
        <div>
          <p className="eyebrow">CAMPUSBITE MANAGEMENT</p>
          <h1>Manage <span>Menu.</span></h1>
        </div>

        <button className="cart-button" onClick={onBack}>
          ← Back to menu
        </button>
      </header>

      <section className="menu-section">
        <div className="section-heading">
          <div>
            <p className="eyebrow">MENU ITEMS</p>
            <h2>{editingId ? `Editing #${editingId}` : "Add a food item"}</h2>
          </div>

          <button className="add-button tool" onClick={onRefresh}>
            Refresh menu
          </button>
        </div>

        <div className="admin-menu-layout">
          <form className="auth-card food-form" onSubmit={handleSubmit}>
            {formError && (
              <p role="alert" className="order-message error">
                {formError}
              </p>
            )}
            {message && <p role="status" className="order-message">{message}</p>}

            {!editingId && (
              <>
                <label htmlFor="food-id">Id</label>
                <input
                  id="food-id"
                  type="number"
                  value={form.id}
                  onChange={(event) => update("id", event.target.value)}
                  placeholder="e.g. 105"
                />
              </>
            )}

            <label htmlFor="food-name">Name</label>
            <input
              id="food-name"
              type="text"
              value={form.name}
              onChange={(event) => update("name", event.target.value)}
              placeholder="e.g. Garlic Bread"
            />

            <div className="form-row">
              <div>
                <label htmlFor="food-category">Category</label>
                <input
                  id="food-category"
                  type="text"
                  value={form.category}
                  onChange={(event) => update("category", event.target.value)}
                  placeholder="e.g. Snacks"
                />
              </div>

              <div>
                <label htmlFor="food-price">Price (₹)</label>
                <input
                  id="food-price"
                  type="number"
                  min="0"
                  value={form.price}
                  onChange={(event) => update("price", event.target.value)}
                  placeholder="e.g. 50"
                />
              </div>
            </div>

            <div className="form-row">
              <div>
                <label htmlFor="food-emoji">Emoji</label>
                <input
                  id="food-emoji"
                  type="text"
                  value={form.emoji}
                  onChange={(event) => update("emoji", event.target.value)}
                  placeholder="🍞"
                />
              </div>

              <div>
                <label htmlFor="food-color">Colour</label>
                <input
                  id="food-color"
                  type="text"
                  value={form.color}
                  onChange={(event) => update("color", event.target.value)}
                  placeholder="#FFF0D8"
                />
              </div>
            </div>

            <label htmlFor="food-description">Description</label>
            <input
              id="food-description"
              type="text"
              value={form.description}
              onChange={(event) => update("description", event.target.value)}
              placeholder="Short description"
            />

            <label className="checkbox-row" htmlFor="food-available">
              <input
                id="food-available"
                type="checkbox"
                checked={form.available}
                onChange={(event) => update("available", event.target.checked)}
              />
              <span>Available for ordering</span>
            </label>

            <div className="form-actions">
              <button className="checkout-button" type="submit" disabled={busy}>
                {busy
                  ? "Saving..."
                  : editingId
                    ? "Save changes"
                    : "Add item"}
              </button>

              {editingId && (
                <button type="button" className="add-button tool" onClick={resetForm}>
                  Cancel
                </button>
              )}
            </div>
          </form>

          <div className="admin-menu-list">
            {loading && <p>Loading menu...</p>}
            {error && <p role="alert">{error}</p>}

            {!loading &&
              !error &&
              foods.map((food) => (
                <article className="menu-row" key={food.id}>
                  <span className="menu-row-emoji" style={{ backgroundColor: food.color }}>
                    {food.emoji || "🍽️"}
                  </span>

                  <div className="menu-row-info">
                    <h3>
                      {food.name} <small>#{food.id}</small>
                    </h3>
                    <p>
                      {food.category} · ₹{food.price}
                      {!food.available && <span className="tag-off">Unavailable</span>}
                    </p>
                  </div>

                  <div className="menu-row-actions">
                    <button className="add-button tool" onClick={() => startEdit(food)}>
                      Edit
                    </button>
                    <button
                      className="add-button tool danger"
                      onClick={() => handleDelete(food)}
                    >
                      Delete
                    </button>
                  </div>
                </article>
              ))}
          </div>
        </div>
      </section>
    </main>
  );
}

export default App;
