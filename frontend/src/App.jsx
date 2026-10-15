
import { useEffect, useMemo, useState } from "react";
import "./App.css";

const categories = ["All", "Burgers", "Pizza", "Snacks", "Drinks"];

function App() {
  const [foods, setFoods] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState("");
  const [activeCategory, setActiveCategory] = useState("All");
  const [search, setSearch] = useState("");
  const [cart, setCart] = useState([]);
  const [showCart, setShowCart] = useState(false);
  const [customerName, setCustomerName] = useState("");
  const [orderMessage, setOrderMessage] = useState("");
  const [submitting, setSubmitting] = useState(false);

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

  useEffect(() => {
    fetch("http://localhost:5000/api/food")
      .then((response) => {
        if (!response.ok) {
          throw new Error("Failed to load food menu");
        }
        return response.json();
      })
      .then((data) => {
        setFoods(data);
        setError("");
      })
      .catch(() => {
        setError("Unable to load the menu. Please check the backend.");
      })
      .finally(() => {
        setLoading(false);
      });
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
      const response = await fetch("http://localhost:5000/api/orders", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          customerName: customerName.trim(),
          items: cart.map((item) => ({
            id: item.id,
            quantity: item.quantity,
          })),
        }),
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
    } catch (error) {
      setOrderMessage(
        error.message || "Could not connect to the backend."
      );
    } finally {
      setSubmitting(false);
    }
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
                  Checkout is a frontend demo for now.
                </p>
              </>
            )}
          </section>
        </div>
      )}
    </div>
  );
}

export default App;
