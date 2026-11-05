const dns = require('dns');
const mongoose = require('mongoose');

// Some home routers / ISPs refuse the DNS SRV query that "mongodb+srv://" needs,
// which surfaces in Node as "querySrv ECONNREFUSED" even though the OS resolver
// is fine. Prepending public resolvers lets c-ares resolve the SRV record, while
// keeping any existing (e.g. router) servers for local names.
dns.setServers([...new Set(['8.8.8.8', '1.1.1.1', ...dns.getServers()])]);

const connectDB = async () => {
    try {
        await mongoose.connect(process.env.MONGO_URI, {
            serverSelectionTimeoutMS: 10000,
        });

        console.log("MongoDB Connected Successfully!");
        return true;
    }
    catch (error) {
        // Non-fatal: the canteen data lives in the C++ core (data/*.txt), so the
        // API must still start even if MongoDB is unreachable.
        console.log("MongoDB Connection Failed:", error.message);
        return false;
    }
};

module.exports = connectDB;
