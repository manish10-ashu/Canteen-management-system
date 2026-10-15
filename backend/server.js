
const express = require('express');
const cors = require('cors');

const app = express();
const PORT = 5000;
let orders = [];
let nextOrderId = 1001;

app.use(cors());
app.use(express.json());

// Food menu
const foodItems = [
    {
        id: 101,
        name: "Classic Burger",
        category: "Burgers",
        price: 80,
        emoji: "🍔",
        description: "Crispy, juicy and freshly prepared",
        color: "#FFF0D8"
    },
    {
        id: 102,
        name: "Cheese Pizza",
        category: "Pizza",
        price: 150,
        emoji: "🍕",
        description: "Cheesy goodness in every bite",
        color: "#FCE5D7"
    },
    {
        id: 103,
        name: "Steamed Momos",
        category: "Snacks",
        price: 60,
        emoji: "🥟",
        description: "Soft dumplings with spicy chutney",
        color: "#E4F1DE"
    },
    {
        id: 104,
        name: "Cold Coffee",
        category: "Drinks",
        price: 70,
        emoji: "🥤",
        description: "Chilled, creamy and refreshing",
        color: "#E5E9FF"
    }
];

// Test API
app.get('/api/health', (req, res) => {
    res.json({
        success: true,
        message: 'Canteen backend is running!'
    });
});

// Get all food items
app.get('/api/food', (req, res) => {
    res.json(foodItems);
});

app.post('/api/orders', (req, res) => {
    const { customerName, items } = req.body;

    if (!customerName || !customerName.trim()) {
        return res.status(400).json({
            success: false,
            message: 'Customer name is required.'
        });
    }

    if (!Array.isArray(items) || items.length === 0) {
        return res.status(400).json({
            success: false,
            message: 'Your cart is empty.'
        });
    }

    const orderItems = [];

    for (const item of items) {
        const food = foodItems.find(
            (food) => food.id === Number(item.id)
        );

        const quantity = Number(item.quantity);

        if (!food || !Number.isInteger(quantity) ||
            quantity < 1 || quantity > 100) {
            return res.status(400).json({
                success: false,
                message: 'Invalid food item or quantity.'
            });
        }

        orderItems.push({
            id: food.id,
            name: food.name,
            price: food.price,
            quantity: quantity
        });
    }

    const total = orderItems.reduce(
        (sum, item) => sum + item.price * item.quantity,
        0
    );

    const order = {
        id: nextOrderId++,
        customerName: customerName.trim(),
        items: orderItems,
        total: total,
        status: 'Pending',
        createdAt: new Date().toISOString()
    };

    orders.push(order);

    res.status(201).json({
        success: true,
        message: 'Order placed successfully!',
        order: order
    });
});

// Start server
app.listen(PORT, () => {
    console.log(`Backend running at http://localhost:${PORT}`);
});
