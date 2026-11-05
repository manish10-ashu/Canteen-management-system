// ---------------------------------------------------------------------------
//  Mongoose models for the canteen data.
//
//  The C++ core owns the logic (linked lists, FIFO queue, history), but it is
//  stateless, so the documents here are the durable copy of that state. Each
//  request loads them, hands them to the core, and writes the result back.
// ---------------------------------------------------------------------------

const mongoose = require('mongoose');

const foodSchema = new mongoose.Schema(
  {
    id: { type: Number, required: true, unique: true },
    name: { type: String, default: '' },
    category: { type: String, default: '' },
    price: { type: Number, default: 0 },
    available: { type: Boolean, default: true },
    emoji: { type: String, default: '' },
    description: { type: String, default: '' },
    color: { type: String, default: '' },
  },
  { versionKey: false }
);

const customerSchema = new mongoose.Schema(
  {
    id: { type: Number, required: true, unique: true },
    name: { type: String, default: '' },
    phone: { type: String, default: '' },
  },
  { versionKey: false }
);

const orderItemSchema = new mongoose.Schema(
  {
    id: { type: Number, default: 0 },
    name: { type: String, default: '' },
    price: { type: Number, default: 0 },
    quantity: { type: Number, default: 0 },
  },
  { _id: false }
);

const orderSchema = new mongoose.Schema(
  {
    id: { type: Number, required: true, unique: true },
    customerId: { type: Number, default: 0 },
    customerName: { type: String, default: '' },
    total: { type: Number, default: 0 },
    status: { type: String, default: 'Pending' },
    items: { type: [orderItemSchema], default: [] },
  },
  { versionKey: false }
);

module.exports = {
  Food: mongoose.model('Food', foodSchema),
  Customer: mongoose.model('Customer', customerSchema),
  Order: mongoose.model('Order', orderSchema),
};
