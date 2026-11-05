// ---------------------------------------------------------------------------
//  MongoDB persistence for the canteen's shared state.
//
//  The C++ core is stateless and keeps no files, so the backend loads the
//  whole dataset from MongoDB, streams it into the core, and writes back
//  whatever the core returns.
// ---------------------------------------------------------------------------

const mongoose = require('mongoose');

const { Food, Customer, Order } = require('./models');

// The menu the database is seeded with the first time it is used.
const DEFAULT_MENU = [
  {
    id: 101,
    name: 'Classic Burger',
    category: 'Burgers',
    price: 80,
    available: true,
    emoji: '🍔',
    description: 'Crispy, juicy and freshly prepared',
    color: '#FFF0D8',
  },
  {
    id: 102,
    name: 'Cheese Pizza',
    category: 'Pizza',
    price: 150,
    available: true,
    emoji: '🍕',
    description: 'Cheesy goodness in every bite',
    color: '#FCE5D7',
  },
  {
    id: 103,
    name: 'Steamed Momos',
    category: 'Snacks',
    price: 60,
    available: true,
    emoji: '🥟',
    description: 'Soft dumplings with spicy chutney',
    color: '#E4F1DE',
  },
  {
    id: 104,
    name: 'Cold Coffee',
    category: 'Drinks',
    price: 70,
    available: true,
    emoji: '🥤',
    description: 'Chilled, creamy and refreshing',
    color: '#E5E9FF',
  },
];

// 1 === connected (see mongoose.connection.readyState).
function isDbReady() {
  return mongoose.connection.readyState === 1;
}

// Read the complete dataset the core needs to rebuild its structures.
async function loadState() {
  const [food, customers, orders] = await Promise.all([
    Food.find({}).sort({ id: 1 }).lean(),
    Customer.find({}).sort({ id: 1 }).lean(),
    Order.find({}).sort({ id: 1 }).lean(),
  ]);

  return { food, customers, orders };
}

// Make a collection match `docs` exactly: upsert every document and drop the
// ones the core no longer knows about.
async function syncCollection(Model, docs, key) {
  const ids = docs.map((doc) => doc[key]);

  await Model.deleteMany({ [key]: { $nin: ids } });

  if (docs.length === 0) {
    return;
  }

  await Model.bulkWrite(
    docs.map((doc) => ({
      updateOne: {
        filter: { [key]: doc[key] },
        update: { $set: doc },
        upsert: true,
      },
    }))
  );
}

async function saveState(state) {
  const food = state && Array.isArray(state.food) ? state.food : [];
  const customers =
    state && Array.isArray(state.customers) ? state.customers : [];
  const orders = state && Array.isArray(state.orders) ? state.orders : [];

  await syncCollection(Food, food, 'id');
  await syncCollection(Customer, customers, 'id');
  await syncCollection(Order, orders, 'id');
}

// Seed the default menu the very first time the database is used (only when
// every collection is still empty, so an intentionally emptied menu is kept).
async function seedIfEmpty() {
  const [foodCount, customerCount, orderCount] = await Promise.all([
    Food.estimatedDocumentCount(),
    Customer.estimatedDocumentCount(),
    Order.estimatedDocumentCount(),
  ]);

  if (foodCount || customerCount || orderCount) {
    return false;
  }

  await Food.insertMany(DEFAULT_MENU);
  console.log('Seeded the default canteen menu into MongoDB.');

  return true;
}

module.exports = { isDbReady, loadState, saveState, seedIfEmpty, DEFAULT_MENU };
