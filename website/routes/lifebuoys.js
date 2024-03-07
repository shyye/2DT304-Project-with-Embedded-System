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

    let filters = {}
    if (req.query.search != null && req.query.search !== '') {
        filters.search = new RegExp(req.query.search, 'i')
    }
    try {

        // TODO: Search for lifebuoys
        // https://www.youtube.com/watch?v=esy4nRuShl8&list=PLZlA0Gpn_vH8jbFkBjOuFjhxANC63OmXM&index=7
        // At 29:13 in the video
        // and the next video
        const lifebuoys = await Lifebuoy.find(filters)

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
        // res.redirect(`lifebuoys/${lifebuoy.id}`) TODO: Redricit in video
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
// Video #5 https://youtu.be/UIf1Lh9OZ-k?si=xF2h25RqaUsT-jbp
//  Update
router.get('/:id', (req, res) => {
    res.send(`Show lifebuoy with id: ${req.params.id}`)
})

router.get('/:id/edit', async (req, res) => {
    try {
        const lifebuoy = await Lifebuoy.findById(req.params.id)
        res.render('lifebuoys/edit', { lifebuoy: lifebuoy })
    } catch (error) {
        res.redirect('/lifebuoys')
    }   
})

// Update
router.put('/:id', async (req, res) => {
    let lifebuoy
    try {
        lifebuoy = await Lifebuoy.findById(req.params.id)
        lifebuoy.deviceId = req.body.deviceId,
        lifebuoy.zone = req.body.zone,
        lifebuoy.latitude = req.body.latitude,
        lifebuoy.longitude = req.body.longitude,
        lifebuoy.manufactureDate = req.body.manufactureDate,
        lifebuoy.expirationDate = req.body.expirationDate,
        lifebuoy.status = req.body.status
        await lifebuoy.save()

        // res.redirect(`/lifebuoys/${lifebuoy.id}`)
        res.redirect(`/lifebuoys`)
        
    } catch (error) {
        if (lifebuoy == null) {
            res.redirect('/')
        } else {
            res.render('lifebuoys/new', {
                lifebuoy: req.body,
                errorMessage: 'Error updating lifebuoy'
            })
        }             
    }
    // res.send(`Update lifebuoy with id: ${req.params.id}`)
})

// Delete
router.delete('/:id', (req, res) => {
    res.send(`Delete lifebuoy with id: ${req.params.id}`)
})

// Temp function
// TODO: Test different error scenarios, example lat/long
async function addLifebuoy() {
    console.log('Adding lifebuoy');

    const lifebuoy = await Lifebuoy.create(data[2])
    console.log(lifebuoy);
}

module.exports = router