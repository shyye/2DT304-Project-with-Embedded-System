/**
 * Main routes
 */

const express = require('express')
const router = express.Router()

router.get('/', (req, res) => {
    res.redirect('/lifebuoys')
})

router.get('/test', (req, res) => {
    res.send('Hello World TEST');
})

module.exports = router