/**
 * Lifebuoy routes
 */
const express = require('express')
const router = express.Router()
const Lifebuoy = require('../models/lifebuoyModel')

// Temp data
const data = require('../lifebuoys.json')

// Display all lifebuoys
router.get('/', async (req, res) => {
    try {

        // TODO: Search for lifebuoys
        const lifebuoys = await Lifebuoy.find({})

        // Convert Zone value to name
        res.status(200).render('lifebuoys/index', { 
            lifebuoys: lifebuoys,
            searchValue: req.query.search
        })

    } catch (error) {
        res.status(500).json({ message: error.message }) //TODO: Kolla upp message
    }
})

// Display form to create new lifebuoy
router.get('/new', async (req, res) => {
    // res.send('Create lifebuoy') TODO:
    res.render('lifebuoys/new', { lifebuoy: new Lifebuoy() })
})

// TODO: test
router.post('/create', async (req, res) => {

    const lifebuoy = new Lifebuoy({
        deviceId: req.body.deviceId,
        zone: req.body.zone,
        latitude: req.body.latitude,
        longitude: req.body.longitude,
        manufactureDate: req.body.manufactureDate,
        expirationDate: req.body.expirationDate,
        status: req.body.status
    })

    try {
        const newLifebuoy = await lifebuoy.save()

        // TODO: Do everything herer or above
        // const lifebuoy = await Lifebuoy.create({
        //     deviceId: req.body.deviceId,
        //     zone: req.body.zone,
        //     latitude: req.body.latitude,
        //     longitude: req.body.longitude,
        //     manufactureDate: req.body.manufactureDate,
        //     expirationDate: req.body.expirationDate,
        //     status: req.body.status
        // })
  
        // res.status(201).json({ message: "Lifebuoy created", lifebuoy: lifebuoy })
        // res.redirect(`lifebuoys/${lifebuoy.id}`)
        res.redirect('/lifebuoys')
        
    } catch (error) {
        res.render('lifebuoys/new', {
            lifebuoy: req.body,
            errorMessage: 'Error creating lifebuoy'
        })            
    }

    // TODO:
    // try {
    //     addLifebuoy();

    //     // const lifebuoy = await Lifebuoy.create(req.body);
    //     res.status(201).json({ "test": "Hello test, lifebuoy created"})
    // } catch (error) {
    //     res.status(400).json({ message: error.message })
    // }
})


// TODO: 
router.route('/:id')
    .get((req, res) => {
        res.send(`Get lifebuoy with id: ${req.params.id}`)
    })
    .put((req, res) => {
        res.send(`Get lifebuoy with id: ${req.params.id}`)
    })
    .delete((req, res) => {
        res.send(`Get lifebuoy with id: ${req.params.id}`)
    })

// Temp function
// TODO: Test different error scenarios, example lat/long
async function addLifebuoy() {
    console.log('Adding lifebuoy');

    const lifebuoy = await Lifebuoy.create(data[2])
    console.log(lifebuoy);
}

module.exports = router