// ---------------------------------------------------------------------------
//  Canteen Management System - Express API.
//
//  This file is a THIN ADAPTER. Every canteen operation (menu, cart, order
//  queue, history, file persistence) is performed by the C++ core through
//  cppBridge. There is intentionally no food/order/queue logic here.
// ---------------------------------------------------------------------------
// Load backend/.env explicitly (not cwd-relative) so it works whether the
// server is started from the project root or from the backend folder.
require('dotenv').config({ path: require('path').join(__dirname, '.env') });

const connectDB = require('./config/db');
const express = require('express');
const cors = require('cors');
const crypto = require('crypto');

const { runCore, isCoreAvailable, CoreError } = require('./cppBridge');
const { seedIfEmpty } = require('./stateStore');

const app = express();
const PORT = process.env.PORT || 5000;

const ORDER_STATUSES = ['Pending', 'Preparing', 'Delivered'];
const TOKEN_TTL_MS = 2 * 60 * 60 * 1000; // 2 hours
const PHONE_PATTERN = /^[0-9]{6,15}$/;
const COLOR_PATTERN = /^#[0-9a-fA-F]{6}$/;

app.disable('x-powered-by');
app.use(cors());
app.use(express.json({ limit: '100kb' }));

// ==================== SESSION STORE (in-memory) ====================
// Because the core is stateless, the API owns authentication. Tokens live in
// memory only, so a backend restart simply requires logging in again.

const sessions = new Map();

function issueToken(payload) {
  const token = crypto.randomBytes(24).toString('hex');
  sessions.set(token, { ...payload, expiresAt: Date.now() + TOKEN_TTL_MS });
  return token;
}

function getSession(token) {
  if (!token) {
    return null;
  }

  const session = sessions.get(token);

  if (!session) {
    return null;
  }

  if (session.expiresAt < Date.now()) {
    sessions.delete(token);
    return null;
  }

  return session;
}

setInterval(() => {
  const now = Date.now();

  for (const [token, session] of sessions) {
    if (session.expiresAt < now) {
      sessions.delete(token);
    }
  }
}, 10 * 60 * 1000).unref();

// ==================== HELPERS ====================

function asyncRoute(handler) {
  return (req, res, next) => {
    Promise.resolve(handler(req, res, next)).catch(next);
  };
}

function fail(res, status, message) {
  return res.status(status).json({ success: false, message });
}

function isNonEmptyString(value) {
  return typeof value === 'string' && value.trim().length > 0;
}

function toPositiveInt(value) {
  const number = Number(value);
  return Number.isInteger(number) && number > 0 ? number : null;
}

function bearerToken(req) {
  const header = req.get('authorization') || '';
  const match = header.match(/^Bearer\s+(.+)$/i);
  return match ? match[1].trim() : null;
}

function requireAdmin(req, res, next) {
  const session = getSession(bearerToken(req));

  if (!session || session.role !== 'admin') {
    return fail(res, 401, 'Admin authentication required.');
  }

  req.session = session;
  next();
}

function requireCustomerOrAdmin(req, res, next) {
  const session = getSession(bearerToken(req));

  if (!session) {
    return fail(res, 401, 'Authentication required.');
  }

  const requestedId = toPositiveInt(req.params.id);

  if (session.role === 'customer' && session.customerId !== requestedId) {
    return fail(res, 403, 'You can only view your own orders.');
  }

  req.session = session;
  next();
}

// Optional phone -> customer id (0 means a walk-in/guest customer).
async function resolveCustomerId(customerName, phone) {
  if (!phone) {
    return 0;
  }

  const login = await runCore('customer-login', {
    name: customerName,
    phone,
  });

  return login && login.customer ? login.customer.id : 0;
}

// ==================== HEALTH ====================

app.get('/api/health', (req, res) => {
  res.json({
    success: true,
    message: 'Canteen backend is running!',
    coreAvailable: isCoreAvailable(),
  });
});

// ==================== FOOD (public read) ====================

app.get(
  '/api/food',
  asyncRoute(async (req, res) => {
    const foods = await runCore('list-food');
    res.json(Array.isArray(foods) ? foods : []);
  })
);

// ==================== ORDERS (public create) ====================

app.post(
  '/api/orders',
  asyncRoute(async (req, res) => {
    const body = req.body || {};

    if (!isNonEmptyString(body.customerName)) {
      return fail(res, 400, 'Customer name is required.');
    }

    const customerName = body.customerName.trim();

    if (customerName.length > 60) {
      return fail(res, 400, 'Customer name is too long.');
    }

    if (!Array.isArray(body.items) || body.items.length === 0) {
      return fail(res, 400, 'Your cart is empty.');
    }

    if (body.items.length > 50) {
      return fail(res, 400, 'Too many items in the cart.');
    }

    const items = [];

    for (const item of body.items) {
      const id = toPositiveInt(item && item.id);
      const quantity = toPositiveInt(item && item.quantity);

      if (!id || !quantity || quantity > 100) {
        return fail(res, 400, 'Invalid food item or quantity.');
      }

      items.push({ id, quantity });
    }

    let phone = null;

    if (body.phone !== undefined && body.phone !== null && body.phone !== '') {
      if (typeof body.phone !== 'string' || !PHONE_PATTERN.test(body.phone.trim())) {
        return fail(res, 400, 'Please enter a valid phone number.');
      }

      phone = body.phone.trim();
    }

    const customerId = await resolveCustomerId(customerName, phone);

    const result = await runCore('place-order', {
      customerId,
      customerName,
      item: items.map((entry) => `${entry.id}:${entry.quantity}`),
    });

    if (!result.success) {
      return fail(res, 400, result.message || 'Unable to place order.');
    }

    res.status(201).json({
      success: true,
      message: 'Order placed successfully!',
      order: result.order,
    });
  })
);

// ==================== ORDERS (admin) ====================

app.get(
  '/api/orders',
  requireAdmin,
  asyncRoute(async (req, res) => {
    const result = await runCore('list-orders');

    res.json({
      success: true,
      orders: Array.isArray(result.orders) ? result.orders : [],
    });
  })
);

app.patch(
  '/api/orders/:id/status',
  requireAdmin,
  asyncRoute(async (req, res) => {
    const orderId = toPositiveInt(req.params.id);

    if (!orderId) {
      return fail(res, 400, 'Invalid order id.');
    }

    const status = req.body ? req.body.status : undefined;

    if (!ORDER_STATUSES.includes(status)) {
      return fail(res, 400, 'Invalid order status.');
    }

    const result = await runCore('update-order-status', {
      orderId,
      status,
    });

    if (!result.success) {
      return fail(res, 404, result.message || 'Order not found.');
    }

    res.json({
      success: true,
      message: 'Order status updated successfully!',
      order: result.order,
    });
  })
);

app.delete(
  '/api/orders/:id',
  requireAdmin,
  asyncRoute(async (req, res) => {
    const orderId = toPositiveInt(req.params.id);

    if (!orderId) {
      return fail(res, 400, 'Invalid order id.');
    }

    const result = await runCore('cancel-order', { orderId });

    if (!result.success) {
      return fail(res, 404, result.message || 'Order not found.');
    }

    res.json({ success: true, message: 'Order cancelled successfully!' });
  })
);

// ==================== FOOD (admin CRUD) ====================

function validateFoodBody(body, { partial }) {
  const values = {};

  if (!partial || body.name !== undefined) {
    if (!isNonEmptyString(body.name)) {
      return { error: 'Food name is required.' };
    }

    if (body.name.trim().length > 60) {
      return { error: 'Food name is too long.' };
    }

    values.name = body.name.trim();
  }

  if (!partial || body.category !== undefined) {
    if (!isNonEmptyString(body.category)) {
      return { error: 'Food category is required.' };
    }

    if (body.category.trim().length > 40) {
      return { error: 'Food category is too long.' };
    }

    values.category = body.category.trim();
  }

  if (!partial || body.price !== undefined) {
    const price = Number(body.price);

    if (!Number.isFinite(price) || price < 0 || price > 100000) {
      return { error: 'A valid price is required.' };
    }

    values.price = price;
  }

  if (body.emoji !== undefined) {
    if (typeof body.emoji !== 'string' || body.emoji.length > 16) {
      return { error: 'Invalid emoji.' };
    }

    values.emoji = body.emoji;
  } else if (!partial) {
    values.emoji = '';
  }

  if (body.description !== undefined) {
    if (typeof body.description !== 'string' || body.description.length > 200) {
      return { error: 'Invalid description.' };
    }

    values.description = body.description;
  } else if (!partial) {
    values.description = '';
  }

  if (body.color !== undefined) {
    if (typeof body.color !== 'string' || !COLOR_PATTERN.test(body.color)) {
      return { error: 'Color must be a hex value like #FFF0D8.' };
    }

    values.color = body.color;
  } else if (!partial) {
    values.color = '#F2F2F2';
  }

  if (body.available !== undefined) {
    values.available = body.available ? 1 : 0;
  } else if (!partial) {
    values.available = 1;
  }

  return { values };
}

app.post(
  '/api/food',
  requireAdmin,
  asyncRoute(async (req, res) => {
    const body = req.body || {};
    const id = toPositiveInt(body.id);

    if (!id) {
      return fail(res, 400, 'A positive numeric id is required.');
    }

    const validation = validateFoodBody(body, { partial: false });

    if (validation.error) {
      return fail(res, 400, validation.error);
    }

    const result = await runCore('add-food', { id, ...validation.values });

    if (!result.success) {
      return fail(res, 409, result.message || 'Unable to add food item.');
    }

    res.status(201).json({
      success: true,
      message: 'Food item added.',
      food: result.food,
    });
  })
);

app.put(
  '/api/food/:id',
  requireAdmin,
  asyncRoute(async (req, res) => {
    const id = toPositiveInt(req.params.id);

    if (!id) {
      return fail(res, 400, 'Invalid food id.');
    }

    const validation = validateFoodBody(req.body || {}, { partial: true });

    if (validation.error) {
      return fail(res, 400, validation.error);
    }

    const result = await runCore('update-food', { id, ...validation.values });

    if (!result.success) {
      return fail(res, 404, result.message || 'Food item not found.');
    }

    res.json({
      success: true,
      message: 'Food item updated.',
      food: result.food,
    });
  })
);

app.delete(
  '/api/food/:id',
  requireAdmin,
  asyncRoute(async (req, res) => {
    const id = toPositiveInt(req.params.id);

    if (!id) {
      return fail(res, 400, 'Invalid food id.');
    }

    const result = await runCore('delete-food', { id });

    if (!result.success) {
      return fail(res, 404, result.message || 'Food item not found.');
    }

    res.json({ success: true, message: 'Food item removed.' });
  })
);

// ==================== AUTH ====================

app.post(
  '/api/admin/login',
  asyncRoute(async (req, res) => {
    const body = req.body || {};

    if (!isNonEmptyString(body.username) || !isNonEmptyString(body.password)) {
      return fail(res, 400, 'Username and password are required.');
    }

    const result = await runCore('admin-login', {
      username: body.username.trim(),
      password: body.password,
    });

    if (!result.success) {
      return fail(res, 401, 'Invalid username or password.');
    }

    res.json({
      success: true,
      token: issueToken({ role: 'admin' }),
      expiresInSeconds: TOKEN_TTL_MS / 1000,
    });
  })
);

app.post(
  '/api/customers/login',
  asyncRoute(async (req, res) => {
    const body = req.body || {};

    if (!isNonEmptyString(body.name)) {
      return fail(res, 400, 'Name is required.');
    }

    if (typeof body.phone !== 'string' || !PHONE_PATTERN.test(body.phone.trim())) {
      return fail(res, 400, 'A valid phone number is required.');
    }

    const name = body.name.trim();
    const phone = body.phone.trim();

    const result = await runCore('customer-login', { name, phone });

    if (!result.success || !result.customer) {
      return fail(res, 400, result.message || 'Unable to sign in.');
    }

    res.json({
      success: true,
      token: issueToken({
        role: 'customer',
        customerId: result.customer.id,
      }),
      customer: result.customer,
      expiresInSeconds: TOKEN_TTL_MS / 1000,
    });
  })
);

app.get(
  '/api/customers/:id/orders',
  requireCustomerOrAdmin,
  asyncRoute(async (req, res) => {
    const result = await runCore('customer-orders', {
      customerId: toPositiveInt(req.params.id),
    });

    res.json({
      success: true,
      pending: Array.isArray(result.pending) ? result.pending : [],
      history: Array.isArray(result.history) ? result.history : [],
    });
  })
);

// ==================== 404 + ERROR HANDLING ====================

app.use((req, res) => {
  fail(res, 404, 'Not found.');
});

app.use((error, req, res, next) => {
  if (error && error.type === 'entity.parse.failed') {
    return fail(res, 400, 'Invalid JSON body.');
  }

  if (error instanceof CoreError) {
    console.error('[core]', error.message);
    return fail(res, error.statusCode, error.message);
  }

  console.error('[api]', error);

  return fail(res, 500, 'Internal server error.');
});

const startServer = async () => {
    const connected = await connectDB();

    if (connected) {
        try {
            await seedIfEmpty();
        } catch (error) {
            console.log('Seeding skipped:', error.message);
        }
    }

    app.listen(PORT, () => {
        console.log(`Backend running at http://localhost:${PORT}`);
        console.log(
            isCoreAvailable()
                ? 'C++ core detected.'
                : 'WARNING: C++ core not built. Run "npm run build:core".'
        );
    });
};

startServer();

module.exports = app;
