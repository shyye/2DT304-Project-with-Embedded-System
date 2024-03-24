const mongooose = require('mongoose')
const { Schema, model } = mongooose

const lifebuoySchema = new Schema(
    {
        deviceId: {
            type: Number,
            required: true
        },
        zone: {
            type: Number,
            required: true
        },
        latitude: {
            type: String,
            required: true
        },
        longitude: {
            type: String,
            required: true
        },
        manufactureDate: {
            type: String,
            required: true
        },
        expirationDate: {
            type: String,
            required: true
        },
        status: {               // E.g. In place (= 1) / Missing (= 0)
            type: Number,
            required: true
        }
    }
)

const Lifebuoy = model('Lifebuoy', lifebuoySchema)
module.exports = Lifebuoy